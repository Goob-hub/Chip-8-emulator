-- For building the sdl version: --

- make sdl (Will output the binary of the chip8 in build/sdl directories)  

-- How to run the sdl version: --

- Example: ./build/sdl/chip8 ./Roms/TETRIS --delayQuirk --memoryQuirk --shiftingQuirk --vfResetQuirk
- First argument is the filepath of the rom you wish to run.

-- List of flags you can add to toggle quirks when running the sdl build of the chip8: --

- --delayQuirk: This flag will ensure the chip8 waits for the current draw instruction render to finish before fetching and executing more opcodes. If this flag is not toggled, the chip8 will continue its fetch/decode cycle regardless if it is still rendering a frame.

- --memoryQuirk: This flag will ensure the I register is incremented by (x + 1) when storing and loading registers to and from memory. I is left unchanged if this flag is not toggled.

- --vfResetQuirk: This flag will ensure the V[F] register is set to 0 when performing &, |, ^ bitwise operations on registers. If this flag is not toggled the V[F] register will remain unchanged during these operations.

- --shiftingQuirk: This flag will ensure that V[x] is set to V[y] when executing the opcodes 8xy6 and 8xyE respectively. If this flag is not toggled then V[x] will not be set to V[y].

-- For building the web version: --

- run npm -i (emcc needs typescript to generate types for the compiled javascript of the chip8)
- npm run build:web (Will use emcc to compile the chip8 code into wasm, javascript, and its related typescript types. Will output into build/web directory.)

-- How to use the web version: --

TODO: To future me, please describe how to use web version 4head. :)