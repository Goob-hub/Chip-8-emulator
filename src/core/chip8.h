#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>
#include "chip8_struct.h"

/**
 * Initializes a CHIP-8 instance.
 *
 * Initializes registers, timers, memory, fonts, and emulator quirks
 * specified by the command-line arguments.
 */
bool chip8_init(chip8_t *chip8, const char **argv, int argc);

/**
 * Loads a CHIP-8 ROM into memory starting at 0x200.
 */
bool chip8_load_rom_filepath(chip8_t *chip8, const char *filepath);

/**
 * Executes a single CHIP-8 CPU cycle.
 */
void chip8_cycle(chip8_t *chip8);

/**
 * Updates the delay and sound timers.
 * Should be called at 60Hz.
 */
void chip8_update_timers(chip8_t *chip8);

/**
 * Updates the state of a CHIP-8 keypad key.
 */
void chip8_update_key_state(chip8_t *chip8, int8_t keyIndex, bool keyValue);

#endif /* CHIP8_H */