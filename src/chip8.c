#include <stdio.h>
#include <time.h>
#include "chip8.h"

// TODO: Next step is to get the display graphics functionality. This comes in the form of the opcode DXYN We have a gfx[64 * 32] array on our chip8 struct. Each value represents whether the pixel is on or off

//x = (index) % width 
//y = (index) / width
//index = (x * width) + y

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

// Draw pixel at x/y coordinates that is 8xN pixels large
void draw(chip8_t *chip8, uint8_t x, uint8_t y, uint8_t width) {
    
}
