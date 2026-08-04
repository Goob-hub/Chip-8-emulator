CFLAGS = -Wall -Wextra -Wpedantic -g -Isrc/core -Isrc/sdl -Isrc/web

CORE_SRC = src/core/chip8.c src/core/chip8_opcodes.c src/core/chip8_fontset.c

sdl: 
	gcc $(CFLAGS) $(CORE_SRC) src/sdl/main.c -o chip8 $(shell pkg-config --cflags --libs sdl3)

web:
	emcc $(CFLAGS) $(CORE_SRC) src/web/main.c -o chip8.html

.PHONY: clean
clean : 	
		rm edit chip8.html	