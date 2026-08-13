// #include <process.h> windows only
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include "chip8_opcodes.h"
#include "chip8_struct.h"
#include "chip8.h"
#include "chip8_fontset.h"

typedef struct {
    uint32_t window_width;
    uint32_t window_height;
    uint32_t window_scale;
} config_t;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} sdl_t;

typedef struct {
    chip8_t chip8;
    config_t config;
    sdl_t sdl;
    double lastCpuTick;
    double lastTimerTick;
    bool isLogging;
} app_t;

static const char *keymap[16] = {
    "KeyX",    // 0
    "Digit1",  // 1
    "Digit2",  // 2
    "Digit3",  // 3
    "KeyQ",    // 4
    "KeyW",    // 5
    "KeyE",    // 6
    "KeyA",    // 7
    "KeyS",    // 8
    "KeyD",    // 9
    "KeyZ",    // A
    "KeyC",    // B
    "Digit4",  // C
    "KeyR",    // D
    "KeyF",    // E
    "KeyV"     // F
};
// Default flags for emulator to run. Rom wont be loaded initially so the first string is blank
static const char *argv[] = {
    "",
    "--delayQuirk",
    "--memoryQuirk",
    "--vfResetQuirk",
    "--shiftingQuirk"
};
static const int argc = 5;
static app_t app = {0};

static bool init_sdl(sdl_t *sdl, const config_t config) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: '%s'\n", SDL_GetError());
        return false;
    }

    sdl->window = SDL_CreateWindow(
        "My chip8 window",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        config.window_width * config.window_scale,
        config.window_height * config.window_scale,
        0
    );

    if(sdl->window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
        return false;
    }
    
    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);

    return true;
}

static void drawPixel(const config_t config, const sdl_t sdl, uint8_t x, uint8_t y) {    
    SDL_FRect r = {
        .x = x * config.window_scale,
        .y = y * config.window_scale,
        .w = config.window_scale,
        .h = config.window_scale
    };

    SDL_RenderFillRectF(sdl.renderer, &r);
}


static void setup_config(config_t *config) {
    *config = (config_t) {
        .window_width = 64,
        .window_height = 32,
        .window_scale = 1
    };
}

static void final_cleanup(const sdl_t sdl) {
    SDL_DestroyWindow(sdl.window);
    SDL_DestroyRenderer(sdl.renderer);
    SDL_Quit();
}

static void render(const config_t config, const sdl_t sdl, chip8_t *chip8) {
    SDL_SetRenderDrawColor(sdl.renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(sdl.renderer);
    SDL_SetRenderDrawColor(sdl.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    
    for (uint16_t i = 0; i < sizeof(chip8->gfx) / sizeof(chip8->gfx[0]); i++)
    {
        uint8_t currentPixel = chip8->gfx[i];
        
        if(currentPixel == 0x01) {
            uint8_t x = (i) % 64;
            uint8_t y = (i) / 64;
            
            drawPixel(config, sdl, x, y);
        }
    }
    
    SDL_RenderPresent(sdl.renderer);
}

static EM_BOOL onKeyDownEvent(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData) {
    chip8_t *chip8 = (chip8_t *)userData;
    int8_t keyIndex = -1;

    for (unsigned long i = 0; i < sizeof(keymap) / sizeof(keymap[0]); i++)
    {
        if(strcmp(keymap[i], keyEvent->code) == 0) {
            keyIndex = i;
            break;
        }
    }

    if(keyIndex == -1) {
        printf("Unrecognized key pressed.\n");
        return EM_FALSE;
    }

    chip8_update_key_state(chip8, keyIndex, true);

    return EM_TRUE;
}

static EM_BOOL onKeyUpEvent(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData) {
    chip8_t *chip8 = (chip8_t *)userData;   
    int8_t keyIndex = -1;

    for (unsigned long i = 0; i < sizeof(keymap) / sizeof(keymap[0]); i++)
    {
        if(strcmp(keymap[i], keyEvent->code) == 0) {
            keyIndex = i;
            break;
        }
    }

    if(keyIndex == -1) {
        printf("Unrecognized key pressed.\n");
        return EM_FALSE;
    }

    chip8_update_key_state(chip8, keyIndex, false);

    return EM_TRUE;
}

static EM_BOOL one_iteration(double currentTime, void *userData) {
    app_t *app = (app_t *)userData;
    double timerPeriod = 1000 / 60;
    double cpuPeriod = 1000 / app->chip8.cpu_hz;

    if(app->isLogging) {
        printf("frame: %f, pc: %04X\n", currentTime, app->chip8.pc);
    }

    if (app->lastCpuTick == 0) {
        app->lastCpuTick = currentTime;
        app->lastTimerTick = currentTime;
    }

    if (app->chip8.isPaused) {
        render(app->config, app->sdl, &app->chip8);
        return EM_TRUE;
    }

    while(currentTime - app->lastCpuTick >= cpuPeriod) {
        if(!app->chip8.waitForFrame) {
            chip8_cycle(&app->chip8);
        }

        app->lastCpuTick += cpuPeriod;
    }

    while(currentTime - app->lastTimerTick >= timerPeriod) {
        chip8_update_timers(&app->chip8);
        
        app->lastTimerTick += timerPeriod;
    }
    
    render(app->config, app->sdl, &app->chip8);

    return EM_TRUE;
}

// Exported functions that are exposed and can be called from the web

EMSCRIPTEN_KEEPALIVE
bool web_load_rom_chip8(const uint8_t *rom, const unsigned long size) {
    
    bool result = chip8_load_rom_bytes(&app.chip8, rom, size);
    
    if(app.isLogging) {
        printf("Loading ROM: %zu bytes\n", size);
        printf("Load result: %d\n", result);
        printf("First bytes: %02X %02X %02X %02X\n",
            app.chip8.memory[0x200],
            app.chip8.memory[0x201],
            app.chip8.memory[0x202],
            app.chip8.memory[0x203]);
    }

    return result;
}

EMSCRIPTEN_KEEPALIVE
void web_reset_chip8(void) {
    chip8_init(&app.chip8, argv, argc);

    app.lastCpuTick = 0;
    app.lastTimerTick = 0;
}

EMSCRIPTEN_KEEPALIVE
void web_toggle_pause_chip8(void) {
    app.chip8.isPaused = !app.chip8.isPaused;
}

EMSCRIPTEN_KEEPALIVE
void web_toggle_logs(void) {
    app.isLogging = !app.isLogging;
}

int main(void) {

    // TODO: There is a bug in here somewhere. Future me please fix.

    setup_config(&app.config);

    if(!init_sdl(&app.sdl, app.config)) {
        SDL_Quit();
        exit(1);
    }

    if(!chip8_init(&app.chip8, argv, argc)) {
        SDL_Quit();
        exit(1);
    }

    if(app.isLogging) {
        printf("CHIP-8 initialized, PC = %04X\n", app.chip8.pc);
    }

    srand(time(NULL));

    #ifdef __EMSCRIPTEN__
        emscripten_request_animation_frame_loop(one_iteration, &app);
        emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &app.chip8, EM_TRUE, onKeyDownEvent);
        emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &app.chip8, EM_TRUE, onKeyUpEvent);

        return 0;
    #else
        final_cleanup(app.sdl);
        return 0;
    #endif
}