#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "chip8_opcodes.h"
#include "chip8_struct.h"
#include "chip8_fontset.h"

bool chip8_load_rom_filepath(chip8_t *chip8, const char *filepath) {
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

bool chip8_load_rom_bytes(chip8_t *chip8, const uint8_t *rom, const unsigned long size) {
    if(size > sizeof(chip8->memory) - 512) {
        perror("Unable to load rom, file is too large.");
        return false;
    }
    
    memcpy(&chip8->memory[0x200], rom, size);

    return true;
}

bool chip8_init(chip8_t *chip8, const char **argv, const int argc) {
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

    for (uint8_t i = 1; i < argc + 1; i++)
    {
        const char *currentFlag = argv[i];
        
        if(currentFlag && strcmp(currentFlag, "--delayQuirk") == 0) {
            chip8->delayQuirk = true;
        } else if(currentFlag && strcmp(currentFlag, "--memoryQuirk") == 0) {
            chip8->memoryQuirk = true;
        } else if(currentFlag && strcmp(currentFlag, "--vfResetQuirk") == 0) {
            chip8->vfResetQuirk = true;
        } else if(currentFlag && strcmp(currentFlag, "--shiftingQuirk") == 0) {
            chip8->shiftingQuirk = true;
        }
    }
    
    return true;
}

uint16_t fetch_opcode(chip8_t *chip8) {
    // When shifting, the char gets promoted to an int(32bits) to ensure data isnt lost and that there is space neccessary to shift
    // We then convert it back to a short(16bits) to keep data at its relative size
    uint16_t opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];

    chip8->pc += 2;

    return opcode;
}

void decode_opcode(chip8_t *chip8, uint16_t opcode) {
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

void chip8_update_timers(chip8_t *chip8) {
    if(chip8->isPaused) {
        return;
    }

    if(chip8->delay_timer > 0) chip8->delay_timer--;

    if(chip8->sound_timer > 0) chip8->sound_timer--;

    chip8->waitForFrame = false;
}

void chip8_cycle(chip8_t *chip8) {
    if(chip8->isPaused) {
        return;
    }

    uint16_t opcode = fetch_opcode(chip8);
    decode_opcode(chip8, opcode);
}

void chip8_update_key_state(chip8_t *chip8, int8_t keyIndex, bool keyValue) {
    chip8->key[keyIndex] = keyValue;
}

void chip8_reset_state(chip8_t *chip8) {
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;
    chip8->I = 0;
    chip8->sp = 0;
    chip8->pc = 0x200;
    chip8->isPaused = false;

    memset(chip8->V, 0, sizeof(chip8->V));
    memset(chip8->stack, 0, sizeof(chip8->stack));
    memset(chip8->gfx, 0, sizeof(chip8->gfx));
    memset(chip8->key, 0, sizeof(chip8->key));
}