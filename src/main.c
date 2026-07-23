// #include <process.h> windows only
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <SDL3/SDL.h>
#include "chip8.h"
#include "chip8_fontset.h"

// TODO: Keyboard input not working yet, finish implementing that john

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

static bool load_rom(chip8_t *chip8, const char *filepath) {
    FILE* filePointer = fopen(filepath, "rb"); 

    if(filePointer == NULL) {
        perror("Unable to load rom, file does not exist.");
        fclose(filePointer);
        return false;
    }

    fseek(filePointer, 0, SEEK_END); // seek to end of file
    unsigned long size = ftell(filePointer); // get current file pointer

    if(size > sizeof(chip8->memory) - 512) {
        perror("Unable to load rom, file is too large.");
        fclose(filePointer);
        return false;
    }

    fseek(filePointer, 0, SEEK_SET); // seek back to beginning of file
    // proceed with allocating memory and reading the file

    fread(&chip8->memory[0x200], sizeof(uint8_t), size, filePointer);

    fclose(filePointer);

    return true;
}

static void updateKeyState(chip8_t *chip8, SDL_Event *event) {

    SDL_Scancode scancode = event->key.scancode;
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
        case SDL_EVENT_KEY_DOWN:
            chip8->key[keyIndex] = 1;
            break;
        case SDL_EVENT_KEY_UP:
            chip8->key[keyIndex] = 0;
            break;
        default:
            break;
    }
}

static bool init_chip8(chip8_t *chip8, const char **argv) {
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;
    chip8->cpu_hz = 700;
    chip8->pc = 0x200;

    int fontAddress = 0x00;

    for (unsigned long i = 0; i < sizeof(chip8_fontset) / sizeof(chip8_fontset[0]); i++)
    {
        int currentAddress = fontAddress + i;

        chip8->memory[currentAddress] = chip8_fontset[i];
    }

    if(argv[1]) {
        if(!load_rom(chip8, argv[1])) {
            return false;
        }
    }

    if(argv[2] && strcmp(argv[2], "--delayQuirk") == 0) {
        chip8->delayQuirk = true;
    }

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

static uint16_t fetch(chip8_t *chip8) {
    // When shifting, the char gets promoted to an int(32bits) to ensure data isnt lost and that there is space neccessary to shift
    // We then convert it back to a short(16bits) to keep data at its relative size
    uint16_t opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];

    chip8->pc += 2;

    return opcode;
}

static void decode(chip8_t *chip8, uint16_t opcode) {
    uint8_t nibble1 = opcode >> 12;
    uint8_t x = (opcode >> 8) & 0x0F; // nibble2
    uint8_t y = (opcode >> 4) & 0x0F; // nibble3
    uint8_t n = opcode & 0x0F; // nibble4
    uint8_t nn = opcode & 0xFF; // nibble3 && nibble4
    uint16_t nnn = opcode & 0x0FFF;  // nibble2 && nibble3 && nibble4 

    switch (nibble1)
    {
    case 0x00:
        if(nn == 0xE0) {
            clear_screen(chip8);
        } else if(nn == 0xEE) {
            return_subroutine(chip8);
        } 
        break;
    case 0x01:
        jump(chip8, nnn);
        break;
    case 0x02:
        call_subroutine(chip8, nnn);
        break;
    case 0x03:
        skip_if_equal(chip8, x, nn);
        break;
    case 0x04:
        skip_if_not_equal(chip8, x, nn);
        break;
    case 0x05:
        if(n == 0x0) {
            skip_if_registers_equal(chip8, x, y);
        }
        break;
    case 0x06:
        set_register(chip8, x, nn);
        break;
    case 0x07:
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
        skip_if_registers_not_equal(chip8, x, y);
        break;
    case 0x0A:
        set_index_register(chip8, nnn);
        break;
    case 0x0B:
        jump_with_offset(chip8, nnn);
        break;
    case 0x0C:
        set_random_masked_value(chip8, x, nn);
        break;
    case 0x0D:
        draw(chip8, x, y, n);

        if(chip8->delayQuirk) {
            chip8->waitForFrame = true;
        }
        break;
    case 0x0E:
        if(nn == 0x9E) {
            skip_if_key_pressed(chip8, x);
        } else if(nn == 0xA1) {
            skip_if_key_not_pressed(chip8, x);
        }
        break;
    case 0x0F:
        if(nn == 0x1E) {
            add_index_register(chip8, x);
        } else if(nn == 0x0A) {
            await_keypress(chip8, x);
        } else if(nn == 0x29) {
            set_font_character_address(chip8, x);
        } else if(nn == 0x33) {
            binary_decimal_conversion(chip8, x);
        } else if(nn == 0x07) {
            store_delay_timer(chip8, x);
        } else if(nn == 0x15) {
            set_delay_timer(chip8, x);
        } else if(nn == 0x18) {
            set_sound_timer(chip8, x);
        } else if(nn == 0x55) {
            store_registers_to_memory(chip8, x);
        } else if(nn == 0x65) {
            load_registers_from_memory(chip8, x);
        }
        break;
    
    default:
        printf("Unrecognized opcode detected.");
        return;
        break;
    }

    //TODO: Add debugging flag. use argv[2]
    // printf("PC: 0x%03X (%3d)  Opcode: 0x%04X\n",
    //    chip8->pc,
    //    chip8->pc,
    //    opcode);
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

    if(!init_chip8(&chip8, argv)) {
        SDL_Quit();
        exit(1);
    }

    srand(time(NULL));

    while (!done) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                done = true;
            }

            if(event.type == SDL_EVENT_KEY_UP || event.type == SDL_EVENT_KEY_DOWN) {
                updateKeyState(&chip8, &event);
            }
        }  

        uint64_t currentTime = SDL_GetTicks();

        if(currentTime - lastTimerTick >= (1000 / 60)) {
            if(chip8.delay_timer > 0) chip8.delay_timer--;

            if(chip8.sound_timer > 0) chip8.sound_timer--;

            lastTimerTick = currentTime;

            render(config, sdl, &chip8);

            chip8.waitForFrame = false;
        }

        if(currentTime - lastCpuTick >= (1000 / chip8.cpu_hz)) {
            if(!chip8.waitForFrame) {
                uint16_t opcode = fetch(&chip8);
                decode(&chip8, opcode);
    
                lastCpuTick = currentTime;
            }
        }
    }

    final_cleanup(sdl);
    exit(0);
}