#include <stdio.h>
#include <process.h>
#include <stdbool.h>
#include "chip8.h"
#include "../SDL3/SDL.h"

struct chip8 myChip8;

typedef struct {
    uint32_t window_width;
    uint32_t window_height;
} config_t;

typedef struct {
    SDL_Window *window;
} sdl_t;

bool init_sdl(sdl_t *sdl, const config_t config) {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
        printf("Error: SDL initialization failed");
        return false;
    }

    sdl->window = SDL_CreateWindow(
        "My chip8 window",
        config.window_height,
        config.window_width,
        0
    );

    if(sdl->window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

void final_cleanup(window) {
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main() {
    
    sdl_t sdl = {NULL};
    config_t config = {64, 32};

    if(!init_sdl(&sdl, config)) {
        return 1;
    }

    // while (!done) {
    //     SDL_Event event;

    //     while (SDL_PollEvent(&event)) {
    //         if (event.type == SDL_EVENT_QUIT) {
    //             done = true;
    //         }
    //     }
    // }

    final_cleanup(sdl.window);

    return 0;
}