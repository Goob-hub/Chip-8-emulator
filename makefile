CFLAGS = -Wall -Wextra -Wpedantic -g -Isrc/core -Isrc/sdl -Isrc/web

CORE_SRC = src/core/chip8.c src/core/chip8_opcodes.c src/core/chip8_fontset.c

WEB_EXPORTED_CHIP8_FUNCTIONS = '["_chip8_load_rom_bytes"]'

sdl: 
	gcc $(CFLAGS) $(CORE_SRC) src/sdl/main.c -o chip8 $(shell pkg-config --cflags --libs sdl3)

web:
	emcc $(CFLAGS) $(CORE_SRC) src/web/main.c -o chip8.html -sEXPORTED_FUNCTIONS=$(WEB_EXPORTED_CHIP8_FUNCTIONS) \
	-sEXPORTED_RUNTIME_METHODS=ccall,cwrap -sES6_EXPORT=1

.PHONY: clean
clean : 	
		rm edit chip8.html	