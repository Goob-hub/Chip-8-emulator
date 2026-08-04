#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "chip8_struct.h"
#ifndef CHIP8_OPCODES_H
#define CHIP8_OPCODES_H

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