CFLAGS = -Wall -Wextra -Wpedantic -g -Isrc/core -Isrc/sdl -Isrc/web

CORE_SRC = src/core/chip8.c src/core/chip8_opcodes.c src/core/chip8_fontset.c

WEB_EXPORTED_CHIP8_FUNCTIONS = '["_chip8_load_rom_bytes"]'

SDL_BUILD_DIR = build/sdl

WEB_BUILD_DIR = build/web

sdl: 
	mkdir -p $(SDL_BUILD_DIR) 
	gcc $(CFLAGS) $(CORE_SRC) src/sdl/main.c -o $(SDL_BUILD_DIR)/chip8 $(shell pkg-config --cflags --libs sdl3) 

web:
	mkdir -p $(WEB_BUILD_DIR)
	emcc $(CFLAGS) $(CORE_SRC) src/web/main.c -o $(WEB_BUILD_DIR)/chip8.html \
	-sEXPORTED_FUNCTIONS=$(WEB_EXPORTED_CHIP8_FUNCTIONS) -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -sEXPORT_ES6=1 -sUSE_SDL=3

.PHONY: clean
clean : 	
		rm -rf build