# Makefile for ls-v1.0.0 project

# Compiler
CC = gcc

# Directories
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj

# Source and Output
SRC = $(SRC_DIR)/ls-v1.0.0.c
OUT = $(BIN_DIR)/ls-v1.0.0

# Compilation flags
CFLAGS = -Wall

# Default target
all: $(OUT)

# Rule to build the executable
$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

# Clean up build files
clean:
	rm -f $(OUT) $(OBJ_DIR)/*.o
