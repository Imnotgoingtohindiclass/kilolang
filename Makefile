CC      := cc
CFLAGS  := -std=c11 -Wall -Wextra -Isrc
SRC     := $(wildcard src/*.c) $(wildcard src/*/*.c)
OBJ     := $(SRC:src/%.c=build/%.o)
BIN     := bin/kiloc

$(BIN): $(OBJ)
	@mkdir -p bin
	$(CC) $^ -o $@

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build bin

.PHONY: clean all