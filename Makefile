.PHONY: clean build test
CC = gcc
CFLAGS = -Wall $(shell pkg-config --cflags sdl2)
LDFLAGS = $(shell pkg-config --libs sdl2)

test:
	@$(CC) -Wall test/chip8_test.c src/chip8.c -o test/test_chip8
	@cd test && ./test_chip8
	@rm -f test/test_chip8

build:
	$(CC) $(CFLAGS) src/main.c src/chip8.c -o chip8 $(LDFLAGS)

clean:
	rm -f chip8 test/test_chip8
