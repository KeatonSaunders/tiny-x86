CC = gcc
CFLAGS = -Wall -Werror -g
ASM = nasm
ASMFLAGS = -f bin

# Main executable names
TARGET = main
TEST_TARGET = tests

# Assembly binary
ASM_SRC = fib.asm
ASM_BIN = fib.bin

# Source files
EMU_SRC = tiny_x86.c cache.c
MAIN_SRC = main.c
TEST_SRC = tests.c

# Header files
HEADERS = tiny_x86.h cache.h

all: $(TARGET) $(ASM_BIN)

$(TARGET): $(EMU_SRC) $(MAIN_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(EMU_SRC) $(MAIN_SRC)

$(TEST_TARGET): $(EMU_SRC) $(TEST_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(EMU_SRC) $(TEST_SRC)

$(ASM_BIN): $(ASM_SRC)
	$(ASM) $(ASMFLAGS) $< -o $@

run: all
	./$(TARGET) $(ASM_BIN)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	del $(TARGET).exe $(TEST_TARGET).exe $(ASM_BIN)

.PHONY: all run test clean