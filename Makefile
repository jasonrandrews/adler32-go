# Makefile for adler32_neon library and test

# Set the implementation file (can be overridden with make IMPL=adler32_simple.c)
IMPL ?= adler32_neon.c

# Set the compiler explicitly to clang (can be overridden with make CC=gcc)
CC = clang

# Default target
all: compile run

# Compile target to build the libadler32_neon.so
compile:
	@echo "Building with implementation: $(IMPL) using compiler: $(CC)"
	$(CC) -shared -o libadler32_neon.so -fPIC -march=native -O3 -ftree-vectorize -ffast-math -flto $(IMPL)

# Run target to execute the test
run:
	LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH go run adler32.go

# Targets for specific implementations
simple: IMPL=adler32_simple.c
simple: compile

block: IMPL=adler32_block.c
block: compile

neon: IMPL=adler32_neon.c
neon: compile

# Targets for specific compilers
gcc: CC=gcc
gcc: compile

clang: CC=clang
clang: compile

# Clean target to remove generated files
clean:
	rm -f libadler32_neon.so

.PHONY: all compile run clean simple block neon gcc clang
