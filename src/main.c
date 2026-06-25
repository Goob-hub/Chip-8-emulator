// #include <process.h> windows only
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include "chip8.h"

typedef struct {
    uint32_t window_width;
    uint32_t window_height;
    uint32_t window_scale;
} config_t;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *bitmapTexture;
} sdl_t;

bool init_sdl(sdl_t *sdl, const config_t config) {
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

    sdl->bitmapTexture = SDL_CreateTexture(sdl->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, config.window_scale * config.window_width, config.window_scale * config.window_height);

    return true;
}

void init_chip8(chip8_t *chip8) {
    chip8->delay_timer = 60;
    chip8->sound_timer = 60;
    chip8->cpu_hz = 700;
}

void setup_config(config_t *config) {
    *config = (config_t) {
        .window_width = 64,
        .window_height = 32,
        .window_scale = 20
    };
}

void final_cleanup(const sdl_t sdl) {
    SDL_DestroyWindow(sdl.window);
    SDL_DestroyRenderer(sdl.renderer);
    SDL_Quit();
}

void fetch() {

}

void decode() {

}

void render(const config_t config, const sdl_t sdl) {
    // This is how you draw shit in a nutshell    
    SDL_FRect r;

    r.h = 1 * config.window_scale;
    r.w = 1 * config.window_scale;
    r.x = 10 * config.window_scale;
    r.y = 10 * config.window_scale;

    SDL_SetRenderTarget(sdl.renderer, sdl.bitmapTexture);
    SDL_SetRenderDrawColor(sdl.renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(sdl.renderer);
    SDL_RenderRect(sdl.renderer,&r);
    SDL_SetRenderDrawColor(sdl.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderFillRect(sdl.renderer, &r);
    SDL_SetRenderTarget(sdl.renderer, NULL);
    SDL_RenderTexture(sdl.renderer, sdl.bitmapTexture, NULL, NULL);
    SDL_RenderPresent(sdl.renderer);
}

// This function should handle timing with the timers, call the chip8 cycle by decoding opcodes from memory, and call a render function that renders the current chip8's gfx state. 
int main(const int argc, const int **argv) {
    (void)argc;
    (void)argv;
    
    sdl_t sdl = {0};
    config_t config = {0};
    chip8_t chip8 = {0};
    unsigned int lastTime = 0;
    bool done = false;

    setup_config(&config);

    if(!init_sdl(&sdl, config)) {
        SDL_Quit();
        exit(1);
    }

    init_chip8(&chip8);

    while (!done) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                done = true;
            }
        }

        unsigned int currentTime = SDL_GetTicks();

        if(currentTime > lastTime + (1000 / chip8.cpu_hz)) {
            fetch();
            decode();
        }
    }

    final_cleanup(sdl);
    exit(0);
}