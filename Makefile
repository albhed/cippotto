.PHONY: clean build test
CC = gcc
CFLAGS = -Wall $(shell pkg-config --cflags sdl2)
LDFLAGS = $(shell pkg-config --libs sdl2)

test:
	@$(CC) -Wall test/chip8_test.c src/chip8.c src/quirks.c -o test/test_chip8
	@cd test && ./test_chip8
	@rm -f test/test_chip8
	@$(CC) $(CFLAGS) test/display_test.c src/display.c src/chip8.c src/input.c src/quirks.c -o test/test_display $(LDFLAGS)
	@cd test && SDL_VIDEODRIVER=dummy ./test_display
	@rm -f test/test_display
	@$(CC) $(CFLAGS) test/input_test.c src/display.c src/chip8.c src/input.c src/quirks.c -o test/test_input $(LDFLAGS)
	@cd test && SDL_VIDEODRIVER=dummy ./test_input
	@rm -f test/test_input
	@$(CC) $(CFLAGS) test/audio_test.c src/audio.c src/chip8.c src/quirks.c -o test/test_audio $(LDFLAGS)
	@cd test && SDL_AUDIODRIVER=dummy ./test_audio
	@rm -f test/test_audio
	@$(CC) $(CFLAGS) test/loop_test.c src/loop.c src/display.c src/chip8.c src/input.c src/audio.c src/quirks.c -o test/test_loop $(LDFLAGS)
	@cd test && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ./test_loop
	@rm -f test/test_loop
	@$(CC) -Wall test/quirks_test.c src/quirks.c -o test/test_quirks
	@cd test && ./test_quirks
	@rm -f test/test_quirks

build:
	$(CC) $(CFLAGS) src/main.c src/loop.c src/chip8.c src/display.c src/input.c src/audio.c src/quirks.c -o chip8 $(LDFLAGS)

clean:
	rm -f chip8 test/test_chip8 test/test_display test/test_input test/test_audio test/test_loop test/test_quirks
