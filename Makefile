.PHONY: clean build test
CC = gcc
CFLAGS = -Wall $(shell pkg-config --cflags sdl2)
LDFLAGS = $(shell pkg-config --libs sdl2)

test:
	@$(CC) -Wall test/mem_test.c src/mem.c -o test/test_mem
	@cd test && ./test_mem
	@rm -f test/test_mem

build:
	$(CC) $(CFLAGS) src/main.c src/mem.c -o chip8 $(LDFLAGS)

clean:
	rm -f chip8 test/test_mem
