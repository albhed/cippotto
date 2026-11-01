.PHONY: clean build
CC = gcc
CFLAGS = -Wall $(shell pkg-config --cflags sdl2)
LDFLAGS = $(shell pkg-config --libs sdl2)

build:
	$(CC) $(CFLAGS) src/main.c -o chip8 $(LDFLAGS)

clean:
	rm -f chip8