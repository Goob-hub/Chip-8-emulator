#include <stdio.h>
#include <math.h>
#include <time.h>
#include "chip8.h"

// TODO: Next step is to get the display graphics functionality. This comes in the form of the opcode DXYN We have a gfx[64 * 32] (width * height) array on our chip8 struct. Each value represents whether the pixel is on or off

//x = (index) % width 
//y = (index) / width
//index = (y * width) + x

// For every new row with y, thats a new row of 64 x coordinates. Thats how the 2d grid is layed out in 1d memory

// Should set all pixels in display array to 0 (off)
void clear_screen(chip8_t *chip8) {
    for(int i = 0; i < sizeof(chip8->gfx) / sizeof(chip8->gfx[0]); i++) {
        chip8->gfx[i] = 0;
    }

    return;
}

// Should set PC counter to specified memory address (jumping to a point in memory to execute something) pc does not increment in this function
void jump(chip8_t *chip8, uint16_t address) {
    chip8->pc = address;
}

void skip_if_equal(chip8_t *chip8, uint8_t xAddress, uint8_t value) {
    if(chip8->V[xAddress] == value) {
        chip8->pc += 2;
    }
}

void skip_if_not_equal(chip8_t *chip8, uint8_t xAddress, uint8_t value) {
    if(chip8->V[xAddress] != value) {
        chip8->pc += 2;
    }
}

void skip_if_registers_equal(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress) {
    if(chip8->V[xAddress] == chip8->V[yAddress]) {
        chip8->pc += 2;
    }
}

void skip_if_registers_not_equal(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress) {
    if(chip8->V[xAddress] != chip8->V[yAddress]) {
        chip8->pc += 2;
    }
}

void skip_if_key_pressed(chip8_t *chip8, uint8_t xAddress) {
    // VX in this case contains bits which can be read as hexadecimal or decimal, so using them as array indexes will work.
    if(chip8->key[chip8->V[xAddress]] == 1) {
        chip8->pc += 2;
    }   
}

void skip_if_key_not_pressed(chip8_t *chip8, uint8_t xAddress) {
    if(chip8->key[chip8->V[xAddress]] != 1) {
        chip8->pc += 2;
    }   
}

void add_index_register(chip8_t *chip8, uint8_t xAddress) {
    chip8->I += chip8->V[xAddress];
}

void await_keypress(chip8_t *chip8, uint8_t xAddress) {
    bool isKeyPressed = false;
    uint8_t keyIndex = 0;
    //The index of the key represents the actual key pressed, for example index 12 == 0xB in hexadecimal which corresponds to the actual key on the chip8
    for (uint8_t i = 0; i < sizeof(chip8->key) / sizeof(chip8->key[0]); i++)
    {
        if(chip8->key[i] == 1) {
            isKeyPressed = true;
            keyIndex = i;
            break;
        }
    }

    if(isKeyPressed) {
        chip8->V[xAddress] = keyIndex;
    } else {
        chip8->pc -= 2;
    }
    
}

void set_font_character_address(chip8_t *chip8, uint8_t xAddress) {
    // Since my font is allocated at the first 80 memory addresses and is 5 this will set I to the starting point of the font sprite in memory.
    chip8->I = chip8->V[xAddress] * 5;
}

void jump_with_offset(chip8_t *chip8, uint16_t address) {
    chip8->pc = address + chip8->V[0];
}

void set_random_masked_value(chip8_t *chip8, uint8_t xAddress, uint8_t value) {
    chip8->V[xAddress] = value & (uint8_t)rand();
}

void copy_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress) {
    chip8->V[xAddress] = chip8->V[yAddress];
}

void or_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress) {
    chip8->V[xAddress] |= chip8->V[yAddress];
}

void and_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress) {
    chip8->V[xAddress] &= chip8->V[yAddress];
}

void xor_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress) {
    chip8->V[xAddress] ^= chip8->V[yAddress];
}

void binary_decimal_conversion(chip8_t *chip8, uint8_t xAddress) {
    uint8_t value = chip8->V[xAddress];
    uint8_t indexAddress = chip8->I;

    // 156. 1 gets put in I memory address. 5 gets put in I + 1 memory address. 6 gets put in I + 2 memory address.

    chip8->memory[indexAddress] = value / 100;;

    chip8->memory[indexAddress + 1] = (value / 10) % 10;

    chip8->memory[indexAddress + 2] = value % 10; 
}

void store_delay_timer(chip8_t *chip8, uint8_t xAddress) {
    chip8->V[xAddress] = chip8->delay_timer;
}

void set_delay_timer(chip8_t *chip8, uint8_t xAddress) {
    chip8->delay_timer = chip8->V[xAddress];
}

void set_sound_timer(chip8_t *chip8, uint8_t xAddress) {
    chip8->sound_timer = chip8->V[xAddress];
}

void store_registers_to_memory(chip8_t *chip8, uint8_t xAddress) {
    for (uint8_t i = 0; i < xAddress + 1; i++)
    {
        chip8->memory[chip8->I + i] = chip8->V[i];
    }
}

void load_registers_from_memory(chip8_t *chip8, uint8_t xAddress) {
    for (uint8_t i = 0; i < xAddress + 1; i++)
    {
        chip8->V[i] = chip8->memory[chip8->I + i];
    }
}

void add_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress) {
    uint16_t result = chip8->V[xAddress] + chip8->V[yAddress];

    if(result > 255) {
        chip8->V[0xF] = 1;
    } else {
        chip8->V[0xF] = 0;
    }

    chip8->V[xAddress] += chip8->V[yAddress];
}

void subtract_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress) {
    if(chip8->V[xAddress] >= chip8->V[yAddress]) {
        chip8->V[0xF] = 1;
    } else {
        chip8->V[0xF] = 0;
    }

    chip8->V[xAddress] -= chip8->V[yAddress];
}

void reverse_subtract_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress) {
    if(chip8->V[yAddress] >= chip8->V[xAddress]) {
        chip8->V[0xF] = 1;
    } else {
        chip8->V[0xF] = 0;
    }

    chip8->V[xAddress] = chip8->V[yAddress] - chip8->V[xAddress];
}

// Some ROM's that were made for newer chip8 versions expect this to have different functionality. just be weary of that
void set_left_shift_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress) {
    chip8->V[xAddress] = chip8->V[yAddress];

    uint8_t bitShiftedOut = chip8->V[xAddress] >> 7 & 0x01;

    chip8->V[xAddress] <<= 1;

    chip8->V[0xF] = bitShiftedOut;
}

void set_right_shift_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress) {
    chip8->V[xAddress] = chip8->V[yAddress];

    uint8_t bitShiftedOut = chip8->V[xAddress] & 0x01;

    chip8->V[xAddress] >>= 1;

    chip8->V[0xF] = bitShiftedOut;
}

// Here you push the current pc to the stack as it is the return address and then you set the pc to the new address
void call_subroutine(chip8_t *chip8, uint16_t address) {
    if(chip8->sp >= sizeof(chip8->stack) / sizeof(chip8->stack[0])) {
        return;
    }

    chip8->stack[chip8->sp] = chip8->pc;
    chip8->sp++;
    chip8->pc = address;
}

// Should set pc to address at top of stack;
void return_subroutine(chip8_t *chip8) {
    if(chip8->sp == 0) {
        return;
    }

    chip8->sp--;
    chip8->pc = chip8->stack[chip8->sp];
}

// Should set a value to the specified register index
void set_register(chip8_t *chip8, uint8_t index, uint8_t value) {
    chip8->V[index] = value;
}

// Should add to the value at the specified register index
void add_to_register(chip8_t *chip8, uint8_t index, uint8_t value) {
    chip8->V[index] += value;
}

void set_index_register(chip8_t *chip8, uint16_t address) {
    chip8->I = address;
}

void draw(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress, uint8_t spriteHeight) {
    uint8_t x = chip8->V[xAddress];
    uint8_t y = chip8->V[yAddress];
    uint16_t gfxIndex = (y * 64) + x;
    
    // Reset flag register
    chip8->V[0xF] = 0;

    // In this case, the spriteHeight variable represents how many bytes long the sprite is which corresponds to it's actual height.
    for(uint8_t row = 0; row < spriteHeight; row++) {
        uint8_t currentByte = chip8->memory[chip8->I + row];
        uint8_t currentY = y + row;

        // Read each bit of byte and render if in bounds
        if(currentY < 32) {
            for(uint8_t col = 0; col < 8; col++) {
                uint8_t currentX = x + col;
                // Should equal either 0x01 || 0x00 which tells me if the bit is on or off
                uint8_t currentBit = currentByte >> (7 - col) & 0x01;
                
                // Only render if in bounds
                if(currentX < 64) {
                    // If collision between pixels, set flag register to 1
                    if(chip8->gfx[(currentY * 64) + currentX] == 1 && currentBit == 0x01) {
                        chip8->V[0xF] = 1;
                    }

                    chip8->gfx[(currentY * 64) + currentX] ^= currentBit;  
                }
            }
        }
    }
}