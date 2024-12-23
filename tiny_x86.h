#ifndef TINY_X86_H
#define TINY_X86_H

#include <stdint.h>
#include <stdbool.h>
#include "cache.h"

#define MEMORY_SIZE 256
#define FLAG_CARRY 0x01
#define FLAG_ZERO 0x40
#define FLAG_SIGN 0x80

typedef struct
{
    union
    {
        struct
        {
            uint8_t al, ah;
            uint8_t bl, bh;
            uint8_t cl, ch;
            uint8_t dl, dh;
        };
        uint8_t regs[8];
    };
    uint8_t memory[MEMORY_SIZE];
    uint8_t ip;
    uint8_t sp;
    uint8_t flags;
    InstructionCache icache;
} CPU;

void init_cpu(CPU *cpu);
void execute(CPU *cpu, bool verbose);
void run_cpu(CPU *cpu, bool verbose);
int load_program(CPU *cpu, const char *filename, bool verbose);

#endif