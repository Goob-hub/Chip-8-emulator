// #include <process.h> windows only
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <SDL3/SDL.h>
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
} app_t;

const char *keymap[16] = {
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

static bool init_sdl(sdl_t *sdl, const config_t config) {
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)){
        printf("Error: SDL initialization failed");
        return false;
    }

    sdl->window = SDL_CreateWindow(
        "My chip8 window",
        config.window_width * config.window_scale,
        config.window_height * config.window_scale,
        0
    );

    if(sdl->window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
        return false;
    }
    
    sdl->renderer = SDL_CreateRenderer(sdl->window, NULL);

    return true;
}

static void drawPixel(const config_t config, const sdl_t sdl, uint8_t x, uint8_t y) {    
    SDL_FRect r;
    
    r.h = 1 * config.window_scale;
    r.w = 1 * config.window_scale;
    r.x = x * config.window_scale;
    r.y = y * config.window_scale;
    
    SDL_RenderFillRect(sdl.renderer, &r);
}


static void setup_config(config_t *config) {
    *config = (config_t) {
        .window_width = 64,
        .window_height = 32,
        .window_scale = 20
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
        return;
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
        return;
    }

    chip8_update_key_state(chip8, keyIndex, false);

    return EM_TRUE;
}

EM_BOOL one_iteration(double currentTime, void *userData) {
    app_t *app = (app_t *)userData;
    double timerPeriod = 1000 / 60;
    double cpuPeriod = 1000 / app->chip8.cpu_hz;

    while(currentTime - app->lastCpuTick >= cpuPeriod) {
        if(!app->chip8.waitForFrame) {
            chip8_cycle(&app->chip8);
            app->lastCpuTick += cpuPeriod;
        }
    }

    while(currentTime - app->lastTimerTick >= timerPeriod) {
        chip8_update_timers(&app->chip8);
        
        app->lastTimerTick += timerPeriod;
    }
    
    render(app->config, app->sdl, &app->chip8);

    return EM_TRUE;
}

// TODO: Figure out how to load roms differently because its loaded without command line arguments. frontend user can select roms to load into the chip8. Find where you should take in the rom pointer as a parameter and load accordingly. Also set default flags in init
int main() {
    app_t app = {0};

    // Default flags for emulator to run. Rom wont be loaded initially so the first string is blank
    const char *argv = {
        "",
        "--delayQuirk",
        "--memoryQuirk",
        "--vfResetQuirk",
        "--shiftingQuirk"
    };
    const int argc = 5;

    setup_config(&app.config);

    if(!init_sdl(&app.sdl, app.config)) {
        SDL_Quit();
        exit(1);
    }

    if(!chip8_init(&app.chip8, argv, argc)) {
        SDL_Quit();
        exit(1);
    }

    srand(time(NULL));

    #ifdef __EMSCRIPTEN__
        emscripten_request_animation_frame_loop(one_iteration, &app);
        emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &app.chip8, EM_TRUE, onKeyDownEvent);
        emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &app.chip8, EM_TRUE, onKeyUpEvent);

        return 0;
    #else
        final_cleanup(app.sdl);
    #endif
}