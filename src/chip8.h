#include <stdio.h>

typedef struct {
    unsigned char memory[4096]; //Limit for memory that chip-8 programs can use, basically the memory of the program not the chip8, this just specifies the limit.
    unsigned char V[16]; //General cpu registers
    unsigned short I; //Memory address that points to data
    unsigned short pc; //Points at memory location to form opcode. (instruction)
    unsigned char gfx[64 * 32]; //Display graphics specs
    __uint8_t delay_timer; //Ticks 60 times per second, cpu speed runs at full but this is the main tracker of time
    __uint8_t sound_timer; //
    __uint32_t cpu_hz;
    unsigned short stack[16]; //Stores return addresses, where the chip8 goes back to after a call. Be weary of overflow. ignore it or add safety checks
    unsigned short sp; //Index of the next free slot in the stack. increases on call and decreases on return
    unsigned short key[16]; //emulator input state, emulator must map to actual keyboard
} chip8_t;

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