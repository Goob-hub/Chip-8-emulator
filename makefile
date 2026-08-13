CFLAGS = -Wall -Wextra -Wpedantic -g -Isrc/core -Isrc/sdl -Isrc/web
CORE_SRC = src/core/chip8.c src/core/chip8_opcodes.c src/core/chip8_fontset.c

WEB_EXPORTED_CHIP8_FUNCTIONS = '["_main","_web_toggle_logs","_web_load_rom_chip8","_web_reset_chip8","_web_toggle_pause_chip8","_malloc","_free"]'
WEB_EXPORTED_RUNTIME_METHODS = '["ccall","cwrap","HEAPU8"]'

SDL_BUILD_DIR = build/sdl
WEB_BUILD_DIR = build/web

sdl: 
	mkdir -p $(SDL_BUILD_DIR) 
	gcc $(CFLAGS) $(CORE_SRC) src/sdl/main.c -o $(SDL_BUILD_DIR)/chip8 $(shell pkg-config --cflags --libs sdl2) 

web:
	mkdir -p $(WEB_BUILD_DIR)
	emcc $(CFLAGS) $(CORE_SRC) src/web/main.c -o $(WEB_BUILD_DIR)/chip8.js \
	-sEXPORTED_FUNCTIONS=$(WEB_EXPORTED_CHIP8_FUNCTIONS) -sEXPORTED_RUNTIME_METHODS=$(WEB_EXPORTED_RUNTIME_METHODS) -sEXPORT_ES6=1 -sMODULARIZE=1 -sUSE_SDL=2 -sNO_EXIT_RUNTIME=1 --emit-tsd chip8.d.ts 

.PHONY: clean
clean : 	
		rm -rf build