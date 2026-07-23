#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#ifndef CHIP8_H
#define CHIP8_H

// Chip 8 emulator simulates a cpu and interprets a fixed memory layout, it is not dynamic in this case. The memory is interpreted as instructions, the definition here just sets the limit that programs can be/use

//Chip 8 doesnt care what the subroutines are supposed to be calling, it simply interprets memory as instructions

// fetch opcode from memory[PC]
// decode opcode
// execute behavior (which modifies CPU state)
// advance PC

//You implement all op codes in a huge if else chain/switch statement. All opcodes do something explicit.

// 0110 1010 0000 1111 example of op code structure in binary, what i have to decode.

// ✔️ Correct mental model:

// “The CPU is reading a book page by page using a bookmark (PC)”

// Memory = the book
// PC = the bookmark
// Opcode = one sentence (2 bytes)
// Emulator = the reader

// You are not receiving sentences—you are flipping pages yourself.
// The program is not a stream—it is a static byte array, and the PC is just a pointer your emulator moves through that array 2 bytes at a time, interpreting each chunk as an instruction.

typedef struct {
    uint8_t memory[4096]; //Limit for memory that chip-8 programs can use, basically the memory of the program not the chip8, this just specifies the limit.
    uint8_t V[16]; //General cpu registers
    uint16_t I; //Memory address that points to data
    uint16_t pc; //Points at memory location to form opcode. (instruction)
    uint8_t gfx[64 * 32]; //Display graphics specs
    uint8_t delay_timer; //Ticks 60 times per second, cpu speed runs at full but this is the main tracker of time
    uint8_t sound_timer; 
    uint32_t cpu_hz;
    uint16_t stack[16]; //Stores return addresses, where the chip8 goes back to after a call. Be weary of overflow. ignore it or add safety checks
    uint16_t sp; //Index of the next free slot in the stack. increases on call and decreases on return
    uint8_t key[16]; //emulator input state, emulator must map to actual keyboard
    bool delayQuirk;
    bool waitForFrame;
} chip8_t;


/* ============================================================================
 * System / Initialization
 * ========================================================================== */

void clear_screen(chip8_t *chip8);
void set_index_register(chip8_t *chip8, uint16_t address);
void add_index_register(chip8_t *chip8, uint8_t xAddress);

/* ============================================================================
 * Program Flow
 * ========================================================================== */

void jump(chip8_t *chip8, uint16_t address);
void jump_with_offset(chip8_t *chip8, uint16_t address);

void call_subroutine(chip8_t *chip8, uint16_t address);
void return_subroutine(chip8_t *chip8);

/* ============================================================================
 * Conditional Instructions
 * ========================================================================== */

void skip_if_equal(chip8_t *chip8, uint8_t xAddress, uint8_t value);
void skip_if_not_equal(chip8_t *chip8, uint8_t xAddress, uint8_t value);

void skip_if_registers_equal(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress);
void skip_if_registers_not_equal(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress);

void skip_if_key_pressed(chip8_t *chip8, uint8_t xAddress);
void skip_if_key_not_pressed(chip8_t *chip8, uint8_t xAddress);

/* ============================================================================
 * Register Instructions
 * ========================================================================== */

void set_register(chip8_t *chip8, uint8_t index, uint8_t value);
void add_to_register(chip8_t *chip8, uint8_t index, uint8_t value);

void copy_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress);
void or_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress);
void and_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress);
void xor_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress);

void add_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress);
void subtract_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress);
void reverse_subtract_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress);

void set_left_shift_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress);
void set_right_shift_register(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress);

/* ============================================================================
 * Timers
 * ========================================================================== */

void store_delay_timer(chip8_t *chip8, uint8_t xAddress);
void set_delay_timer(chip8_t *chip8, uint8_t xAddress);
void set_sound_timer(chip8_t *chip8, uint8_t xAddress);

/* ============================================================================
 * Memory
 * ========================================================================== */

void store_registers_to_memory(chip8_t *chip8, uint8_t xAddress);
void load_registers_from_memory(chip8_t *chip8, uint8_t xAddress);

void binary_decimal_conversion(chip8_t *chip8, uint8_t xAddress);

void set_font_character_address(chip8_t *chip8, uint8_t xAddress);

/* ============================================================================
 * Input
 * ========================================================================== */

void await_keypress(chip8_t *chip8, uint8_t xAddress);

/* ============================================================================
 * Random
 * ========================================================================== */

void set_random_masked_value(chip8_t *chip8, uint8_t xAddress, uint8_t value);

/* ============================================================================
 * Graphics
 * ========================================================================== */

void draw(chip8_t *chip8, uint8_t xAddress, uint8_t yAddress, uint8_t spriteHeight);

#endif 