.PHONY: clean build test
CC = gcc
CFLAGS = -Wall $(shell pkg-config --cflags sdl2)
LDFLAGS = $(shell pkg-config --libs sdl2)

test:
	@$(CC) -Wall test/chip8_test.c src/chip8.c -o test/test_chip8
	@cd test && ./test_chip8
	@rm -f test/test_chip8
	@$(CC) $(CFLAGS) test/display_test.c src/display.c src/chip8.c -o test/test_display $(LDFLAGS)
	@cd test && SDL_VIDEODRIVER=dummy ./test_display
	@rm -f test/test_display

build:
	$(CC) $(CFLAGS) src/main.c src/chip8.c src/display.c -o chip8 $(LDFLAGS)

clean:
	rm -f chip8 test/test_chip8 test/test_display
