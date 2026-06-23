#include <stdio.h>
#include <time.h>
#include "chip8.h"

chip8_t chip8 = {0};


// TODO: Next step is to get the display graphics functionality. This comes in the form of the opcode DXYN We have a gfx[64 * 32] array on our chip8 struct. Each value represents whether the pixel is on or off

//x = (index) % width ---- y = (index) / width
//index = (x * width) + y

int draw() {
    return 0;
}
