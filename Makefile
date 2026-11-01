.PHONY: clean build test
CC = gcc
CFLAGS = -Wall $(shell pkg-config --cflags sdl2)
LDFLAGS = $(shell pkg-config --libs sdl2)

test:
	@$(CC) test/mem_test.c -o test_mem
	@./test_mem
	@rm -f test_mem

build:
	$(CC) $(CFLAGS) src/main.c -o chip8 $(LDFLAGS)

clean:
	rm -f chip8 test_mem
