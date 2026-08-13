// #include <process.h> windows only
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
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

const SDL_Scancode keymap[16] = {
    SDL_SCANCODE_X, // 0
    SDL_SCANCODE_1, // 1
    SDL_SCANCODE_2, // 2
    SDL_SCANCODE_3, // 3
    SDL_SCANCODE_Q, // 4
    SDL_SCANCODE_W, // 5
    SDL_SCANCODE_E, // 6
    SDL_SCANCODE_A, // 7
    SDL_SCANCODE_S, // 8
    SDL_SCANCODE_D, // 9
    SDL_SCANCODE_Z, // A
    SDL_SCANCODE_C, // B
    SDL_SCANCODE_4, // C
    SDL_SCANCODE_R, // D
    SDL_SCANCODE_F, // E
    SDL_SCANCODE_V  // F
};

static bool init_sdl(sdl_t *sdl, const config_t config) {
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)){
        printf("Error: SDL initialization failed");
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

static void detectKeyboardEvent(chip8_t *chip8, SDL_Event *event) {

    SDL_Scancode scancode = event->key.keysym.scancode;
    int8_t keyIndex = -1;

    for (unsigned long i = 0; i < sizeof(keymap) / sizeof(keymap[0]); i++)
    {
        if(keymap[i] == scancode) {
            keyIndex = i;
            break;
        }
    }

    if(keyIndex == -1) {
        printf("Unrecognized key pressed.\n");
        return;
    }

    switch (event->type)
    {
        case SDL_KEYDOWN:
            chip8_update_key_state(chip8, keyIndex, true);
            break;
        case SDL_KEYUP:
            chip8_update_key_state(chip8, keyIndex, false);
            break;
        default:
            break;
    }
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

int main(const int argc, const char **argv) {
    sdl_t sdl = {0};
    config_t config = {0};
    chip8_t chip8 = {0};
    uint64_t lastCpuTick = 0;
    uint64_t lastTimerTick = 0;
    bool done = false;

    if (argc < 2) {
        printf("Usage: %s <rom>\n", argv[0]);
        return 1;
    }

    setup_config(&config);

    if(!init_sdl(&sdl, config)) {
        SDL_Quit();
        exit(1);
    }

    if(!chip8_init(&chip8, argv, argc)) {
        SDL_Quit();
        exit(1);
    }

    if(!chip8_load_rom_filepath(&chip8, argv[1])) {
        SDL_Quit();
        exit(1);
    }

    srand(time(NULL));

    while (!done) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_Quit) {
                done = true;
            }

            if(event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
                detectKeyboardEvent(&chip8, &event);
            }
        }  

        uint64_t currentTime = SDL_GetTicks();

        if(currentTime - lastTimerTick >= (1000 / 60)) {
            chip8_update_timers(&chip8);

            lastTimerTick = currentTime;
            
            render(config, sdl, &chip8);
        }
        
        if(currentTime - lastCpuTick >= (1000 / chip8.cpu_hz)) {
            if(!chip8.waitForFrame) {
                chip8_cycle(&chip8);
            }
            
            lastCpuTick = currentTime;
        }
    }

    final_cleanup(sdl);
    exit(0);
}