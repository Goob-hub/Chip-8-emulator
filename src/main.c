// #include <process.h> windows only
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include "chip8.h"
#include "chip8.c"

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

// Figure out how to initialize memory with the chosen program.
void init_chip8(chip8_t *chip8) {
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;
    chip8->cpu_hz = 700;
    chip8->pc = 0x200;
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

void render(const config_t config, const sdl_t sdl, chip8_t *chip8) {
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

void drawPixel(const config_t config, const sdl_t sdl, uint8_t x, uint8_t y) {    
    SDL_FRect r;

    r.h = 1 * config.window_scale;
    r.w = 1 * config.window_scale;
    r.x = x * config.window_scale;
    r.y = y * config.window_scale;

    SDL_RenderFillRect(sdl.renderer, &r);
}

uint16_t fetch(chip8_t *chip8) {
    // When shifting, the char gets promoted to an int(32bits) to ensure data isnt lost and that there is space neccessary to shift
    // We then convert it back to a short(16bits) to keep data at its relative size
    uint16_t opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];

    chip8->pc += 2;

    return opcode;
}

void decode(const config_t config, const sdl_t sdl, chip8_t *chip8, uint16_t opcode) {
    uint8_t nibble1 = opcode >> 12;
    uint8_t x = (opcode >> 8) & 0x0F; // nibble2
    uint8_t y = (opcode >> 4) & 0x0F; // nibble3
    uint8_t n = opcode & 0x0F; // nibble4
    uint8_t nn = opcode & 0xFF; // nibble3 && nibble4
    uint16_t nnn = opcode & 0x0FFF;  // nibble2 && nibble3 && nibble4 

    switch (nibble1)
    {
    case 0x00:
        // Logic for opcodes that start with 0

        if(opcode == 0x00E0) {
            clear_screen(chip8);
        } else if(opcode == 0x00EE) {
            return_subroutine(chip8);
        }
        
        // For different version of chip8
        // if((opcode & 0xFFF0) == 0x00C0) { 
        //    scroll_down(n)
        // }   
        
        break;
    case 0x01:
        // For any opcode where the last 3 nibbles are fill in variables, these only have one default function call
        //1nnn
        jump(chip8, nnn);
        break;
    case 0x02:
        //2nnn 
        call_subroutine(chip8, nnn);
        break;
    case 0x03:
         
        break;
    case 0x04:
         
        break;
    case 0x05:
         
        break;
    case 0x06:
        //6xnn
        set_register(chip8, x, nn);
        break;
    case 0x07:
        //7xnn
        add_to_register(chip8, x, nn);
        break;
    case 0x08:
         if(n == 0x00) {
            copy_register(chip8, x, y);
         } else if(n == 0x01) {
            or_register(chip8, x, y);
         } else if(n == 0x02) {
            and_register(chip8, x, y);
         } else if(n == 0x03) {
            xor_register(chip8, x, y);
         } else if(n == 0x04) {
            add_register(chip8, x, y);
         } else if(n == 0x05) {
            subtract_register(chip8, x, y);
         } else if(n == 0x06) {
            set_right_shift_register(chip8, x, y);
         } else if(n == 0x07) {
            reverse_subtract_register(chip8, x, y);
         } else if(n == 0x0E) {
            set_left_shift_register(chip8, x, y);
         }
        break;
    case 0x09:
         
        break;
    case 0x0A:
        //Annn
        set_index_register(chip8, nnn);
        break;
    case 0x0B:
         
        break;
    case 0x0C:
         
        break;
    case 0x0D:
        //Dxyn
        draw(chip8, x, y, n);
        render(config, sdl, chip8);
        break;
    case 0x0E:
         
        break;
    case 0x0F:
         
        break;
    
    default:
        printf("Unrecognized opcode detected.");
        return;
        break;
    }
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

        if(currentTime > lastTime + (1000 / 60)) {
            chip8.delay_timer--;
            chip8.sound_timer--;
        }

        if(currentTime > lastTime + (1000 / chip8.cpu_hz)) {
            uint16_t opcode = fetch(&chip8);
            decode(config, sdl, &chip8, opcode);
        }
    }

    final_cleanup(sdl);
    exit(0);
}