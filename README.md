# Tiny x86 Emulator

A minimal x86 CPU emulator implementing a subset of 8-bit operations. Built as an educational exercise to revisit concepts from CS Primer's Computer Systems module, covering bits & bytes, C programming, x86 assembly, and basic CPU microarchitecture.

## Features

- 8-bit register operations (AL, BL, CL, DL, AH, BH, CH, DH)
- Basic instruction set: MOV, ADD, SUB, INC, DEC, AND, OR, SHL, SHR, JMP, CMP, conditional jumps
- Stack operations: PUSH, POP
- Function calls: CALL, RET
- Instruction cache with hit/miss statistics
- Uses actual x86 opcodes - can run real machine code compiled with NASM

## Example

The project includes a recursive Fibonacci implementation (fib.asm) that computes F(7) using the classic recursive formula:

F(n) = F(n-1) + F(n-2), where F(0) = 0 and F(1) = 1

While horrendously inefficient with O(2^n) complexity, it serves as a good test of the instruction set by exercising function calls, stack operations, and conditional jumps.

```nasm
section .text
global start

start:
    mov al, 7      ; Calculate F(7)
    call fib       ; Result will be in AL
    hlt            ; Stop execution
```

## Usage

On Windows:
```bash
mingw32-make        # Build emulator and compile fib.asm
mingw32-make run    # Run fib.asm through emulator
mingw32-make test   # Run test suite
```

## Limitations

- Only 256 bytes of memory
- No interrupts or I/O
- Limited to 8-bit operations
- No floating point or SIMD/vector instructions
- No memory segmentation
- Single-threaded
- No pipelining or out-of-order execution

This is a purely educational project and not meant to compete with full-featured CPU emulators.

## Acknowledgments

This project was inspired by:
- Oz Nova's CS Primer Computer Systems module
- [Some Assembly Required](https://github.com/hackclub/some-assembly-required)