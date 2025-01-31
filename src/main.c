#include <stdio.h>
#include <process.h>
#include "../SDL3/SDL.h"

struct SDL_Window window;
struct SDL_Renderer renderer;
struct SDL_Texture texture;

int init() {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
        printf("Error: SDL initialization failed");
        return 1;
    }

    return 0;
}

void deinit() {

}

int main() {
    

    return 0;
}