# Makefile for ls project (supports v1.0.0 and v1.5.0)
CC = gcc
CFLAGS = -Wall

SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj

SRC_V1    = $(SRC_DIR)/ls-v1.0.0.c
OUT_V1    = $(BIN_DIR)/ls-v1.0.0

SRC_V1_5  = $(SRC_DIR)/ls-v1.5.0.c
OUT_V1_5  = $(BIN_DIR)/ls-v1.5.0

# default target builds the latest (v1.5.0) and updates bin/ls
all: dirs $(OUT_V1_5) link-latest

# create directories if missing
dirs:
	mkdir -p $(BIN_DIR) $(OBJ_DIR)

# build v1.0.0 (kept for compatibility)
$(OUT_V1): $(SRC_V1) | dirs
	$(CC) $(CFLAGS) $(SRC_V1) -o $(OUT_V1)

# build v1.5.0 (colorized / latest)
$(OUT_V1_5): $(SRC_V1_5) | dirs
	$(CC) $(CFLAGS) $(SRC_V1_5) -o $(OUT_V1_5)

# make bin/ls point to the latest build (copies the file)
# copying rather than symlink keeps behavior consistent across environments and Docker
link-latest: $(OUT_V1_5)
	cp -f $(OUT_V1_5) $(BIN_DIR)/ls
	chmod +x $(BIN_DIR)/ls

.PHONY: clean dist-clean dirs all link-latest v1.0 v1.5

# convenience targets
v1.0: $(OUT_V1)
v1.5: $(OUT_V1_5) link-latest

# remove only built binaries and obj files
clean:
	rm -f $(OUT_V1) $(OUT_V1_5) $(BIN_DIR)/ls
	rm -f $(OBJ_DIR)/*.o || true

# remove everything including bin and obj directories (use with care)
dist-clean: clean
	rm -rf $(BIN_DIR) $(OBJ_DIR)
