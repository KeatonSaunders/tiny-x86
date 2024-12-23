#include "tiny_x86.h"
#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

void init_cpu(CPU *cpu)
{
    memset(cpu, 0, sizeof(CPU));
    cpu->sp = MEMORY_SIZE - 1; // Stack grows downwards
    init_cache(&cpu->icache);
}

void update_flags(CPU *cpu, uint8_t result)
{
    if (result == 0)
    {
        cpu->flags |= FLAG_ZERO;
    }
    else
    {
        cpu->flags &= ~FLAG_ZERO;
    }

    if (result & 0x80)
    {
        cpu->flags |= FLAG_SIGN;
    }
    else
    {
        cpu->flags &= ~FLAG_SIGN;
    }
}

// Get register pointer by ModR/M byte
uint8_t *get_register(CPU *cpu, uint8_t reg_code)
{
    switch (reg_code & 0x07)
    {
    case 0:
        return &cpu->al;
    case 1:
        return &cpu->cl;
    case 2:
        return &cpu->dl;
    case 3:
        return &cpu->bl;
    case 4:
        return &cpu->ah;
    case 5:
        return &cpu->ch;
    case 6:
        return &cpu->dh;
    case 7:
        return &cpu->bh;
    default:
        return NULL;
    }
}

uint8_t fetch_byte(CPU *cpu)
{
    return cache_fetch_byte(&cpu->icache, cpu->memory, cpu->ip++);
}

void log_message(const char *format, bool verbose, ...)
{
    if (verbose)
    {
        va_list args;
        va_start(args, verbose);
        vprintf(format, args);
        va_end(args);
    }
}

void execute(CPU *cpu, bool verbose)
{
    uint8_t opcode = fetch_byte(cpu);
    uint8_t *dest, *src;
    uint8_t modrm, value;

    // Debug output
    log_message("Executing opcode 0x%02X at IP 0x%02X\n", verbose, opcode, cpu->ip - 1);

    switch (opcode)
    {
    case 0xB0: // MOV AL, imm8
    case 0xB1: // MOV CL, imm8
    case 0xB2: // MOV DL, imm8
    case 0xB3: // MOV BL, imm8
    case 0xB4: // MOV AH, imm8
    case 0xB5: // MOV CH, imm8
    case 0xB6: // MOV DH, imm8
    case 0xB7: // MOV BH, imm8
        dest = get_register(cpu, opcode - 0xB0);
        value = fetch_byte(cpu);
        *dest = value;
        log_message("MOV %s, 0x%02X\n", verbose,
                    (opcode - 0xB0 <= 3) ? "AL\0CL\0DL\0BL" + (opcode - 0xB0) * 3 : "AH\0CH\0DH\0BH" + (opcode - 0xB4) * 3,
                    value);
        break;

    case 0x88: // MOV r/m8, r8
        modrm = fetch_byte(cpu);
        dest = get_register(cpu, modrm & 0x07);
        src = get_register(cpu, (modrm >> 3) & 0x07);
        *dest = *src;
        log_message("MOV: Copied 0x%02X between registers\n", verbose, *src);
        break;

    case 0x00: // ADD r/m8, r8
        modrm = fetch_byte(cpu);
        dest = get_register(cpu, modrm & 0x07);
        src = get_register(cpu, (modrm >> 3) & 0x07);
        value = *dest + *src;
        update_flags(cpu, value);
        *dest = value;
        log_message("ADD: Result 0x%02X\n", verbose, value);
        break;

    case 0x2C: // SUB AL, imm8
        value = fetch_byte(cpu);
        cpu->al -= value;
        update_flags(cpu, cpu->al);
        log_message("SUB AL, 0x%02X = 0x%02X\n", verbose, value, cpu->al);
        break;

    case 0x28: // SUB r/m8, r8
        modrm = fetch_byte(cpu);
        dest = get_register(cpu, modrm & 0x07);
        src = get_register(cpu, (modrm >> 3) & 0x07);
        value = *dest - *src;
        update_flags(cpu, value);
        *dest = value;
        break;

    case 0xFE: // INC/DEC r/m8
        modrm = fetch_byte(cpu);
        dest = get_register(cpu, modrm & 0x07);
        if (((modrm >> 3) & 0x07) == 0)
        { // INC
            (*dest)++;
            log_message("INC: Register now 0x%02X\n", verbose, *dest);
        }
        else
        { // DEC
            (*dest)--;
            log_message("DEC: Register now 0x%02X\n", verbose, *dest);
        }
        update_flags(cpu, *dest);
        break;

    case 0xF6: // MUL/DIV/NOT r/m8
        modrm = fetch_byte(cpu);
        src = get_register(cpu, modrm & 0x07);
        switch ((modrm >> 3) & 0x07)
        {
        case 4: // MUL
            value = cpu->al * (*src);
            cpu->al = value & 0xFF;
            cpu->ah = value >> 8;
            break;
        case 6: // DIV
            if (*src == 0)
            {
                log_message("Division by zero\n", verbose);
                exit(1);
            }
            value = (cpu->ah << 8 | cpu->al) / *src;
            cpu->al = value;
            cpu->ah = (cpu->ah << 8 | cpu->al) % *src;
            break;
        case 2: // NOT
            *src = ~(*src);
            break;
        }
        break;

    case 0x20: // AND r/m8, r8
        modrm = fetch_byte(cpu);
        dest = get_register(cpu, modrm & 0x07);
        src = get_register(cpu, (modrm >> 3) & 0x07);
        value = *dest & *src;
        update_flags(cpu, value);
        *dest = value;
        break;

    case 0x08: // OR r/m8, r8
        modrm = fetch_byte(cpu);
        dest = get_register(cpu, modrm & 0x07);
        src = get_register(cpu, (modrm >> 3) & 0x07);
        value = *dest | *src;
        update_flags(cpu, value);
        *dest = value;
        break;

    case 0xD0: // SHR/SHL r/m8, 1
    case 0xD2: // SHR/SHL r/m8, CL
        modrm = fetch_byte(cpu);
        dest = get_register(cpu, modrm & 0x07);
        uint8_t shift = (opcode == 0xD0) ? 1 : cpu->cl;
        if (((modrm >> 3) & 0x07) == 5)
        { // SHR
            *dest >>= shift;
        }
        else if (((modrm >> 3) & 0x07) == 4)
        { // SHL
            *dest <<= shift;
        }
        update_flags(cpu, *dest);
        break;

    case 0xEB: // JMP rel8
        value = fetch_byte(cpu);
        cpu->ip += (int8_t)value;
        break;

    case 0x38: // CMP r/m8, r8
        modrm = fetch_byte(cpu);
        dest = get_register(cpu, modrm & 0x07);
        src = get_register(cpu, (modrm >> 3) & 0x07);
        value = *dest - *src;
        update_flags(cpu, value);
        break;

    case 0x74: // JE rel8
        value = fetch_byte(cpu);
        if (cpu->flags & FLAG_ZERO)
            cpu->ip += (int8_t)value;
        break;

    case 0x75: // JNE rel8
        value = fetch_byte(cpu);
        if (!(cpu->flags & FLAG_ZERO))
            cpu->ip += (int8_t)value;
        break;

    case 0x7F: // JG rel8
        value = fetch_byte(cpu);
        if (!(cpu->flags & FLAG_ZERO) &&
            !(cpu->flags & FLAG_SIGN))
            cpu->ip += (int8_t)value;
        break;

    case 0x7E: // JLE rel8
    {
        int8_t offset = (int8_t)fetch_byte(cpu);
        bool take_jump = (cpu->flags & FLAG_ZERO) || (cpu->flags & FLAG_SIGN);
        if (take_jump)
        {
            cpu->ip += offset;
            log_message("JLE taken to 0x%02X\n", verbose, cpu->ip);
        }
        else
        {
            log_message("JLE not taken\n", verbose);
        }
    }
    break;

    case 0xE8: // CALL rel16
    {
        // Read 16-bit offset in little-endian order
        int16_t offset = fetch_byte(cpu);          // Low byte
        offset |= ((int16_t)fetch_byte(cpu) << 8); // High byte
        uint8_t return_addr = cpu->ip;
        cpu->memory[--cpu->sp] = return_addr;
        cpu->ip += offset;
        log_message("CALL: offset 0x%04X, from 0x%02X to 0x%02X, pushed return addr 0x%02X\n",
                    verbose, (uint16_t)offset, cpu->ip - 3, cpu->ip, return_addr);
    }
    break;

    case 0xC3: // RET
    {
        uint8_t return_addr = cpu->memory[cpu->sp++];
        cpu->ip = return_addr;
        log_message("RET to 0x%02X\n", verbose, cpu->ip);
    }
    break;

    case 0x52: // PUSH DX
        // Push high byte first, then low byte
        cpu->memory[--cpu->sp] = cpu->dh;
        cpu->memory[--cpu->sp] = cpu->dl;
        log_message("PUSH DX: Stored DX (0x%02X%02X) at SP 0x%02X\n",
                    verbose, cpu->dh, cpu->dl, cpu->sp);
        break;

    case 0x50: // PUSH AX
        // Push high byte first, then low byte
        cpu->memory[--cpu->sp] = cpu->ah;
        cpu->memory[--cpu->sp] = cpu->al;
        log_message("PUSH AX: Stored AX (0x%02X%02X) at SP 0x%02X\n",
                    verbose, cpu->ah, cpu->al, cpu->sp);
        break;

    case 0x58: // POP AX
        // Pop low byte first, then high byte
        cpu->al = cpu->memory[cpu->sp++];
        cpu->ah = cpu->memory[cpu->sp++];
        log_message("POP AX: Loaded AX (0x%02X%02X) from SP 0x%02X\n",
                    verbose, cpu->ah, cpu->al, cpu->sp - 2);
        break;

    case 0x5A: // POP DX
        // Pop low byte first, then high byte
        cpu->dl = cpu->memory[cpu->sp++];
        cpu->dh = cpu->memory[cpu->sp++];
        log_message("POP DX: Loaded DX (0x%02X%02X) from SP 0x%02X\n",
                    verbose, cpu->dh, cpu->dl, cpu->sp - 2);
        break;

    case 0x3C: // CMP AL, imm8
        value = fetch_byte(cpu);
        uint8_t result = cpu->al - value;
        update_flags(cpu, result);
        log_message("CMP AL(0x%02X) with 0x%02X, result 0x%02X, flags 0x%02X\n",
                    verbose, cpu->al, value, result, cpu->flags);
        break;

    case 0xF4: // HLT
        log_message("\nProgram halted\n", true);
        log_message("Final register values:\n", true);
        log_message("AL: 0x%02X (%d)\n", true, cpu->al, cpu->al);
        log_message("BL: 0x%02X (%d)\n", true, cpu->bl, cpu->bl);
        log_message("CL: 0x%02X (%d)\n", true, cpu->cl, cpu->cl);
        log_message("DL: 0x%02X (%d)\n", true, cpu->dl, cpu->dl);
        log_message("SP: 0x%02X\n", true, cpu->sp);
        log_message("IP: 0x%02X\n", true, cpu->ip);
        log_message("Flags: 0x%02X\n", true, cpu->flags);
        print_cache_stats(&cpu->icache);
        exit(0);

    default:
        log_message("Unknown opcode: 0x%02X at IP 0x%02X\n", true, opcode, cpu->ip - 1);
        exit(1);
    }
}

void run_cpu(CPU *cpu, bool verbose)
{
    while (1)
    {
        execute(cpu, verbose);
    }
}

int load_program(CPU *cpu, const char *filename, bool verbose)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        perror("Failed to open program file");
        return -1;
    }

    // Get file size
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size > MEMORY_SIZE)
    {
        printf("Program too large for memory (max %d bytes)\n", MEMORY_SIZE);
        fclose(f);
        return -1;
    }

    // Read program into memory
    size_t read = fread(cpu->memory, 1, size, f);
    fclose(f);

    if (verbose)
    {
        printf("Machine code:\n");
        for (int i = 0; i < read; i++)
        {
            printf("0x%02X: 0x%02X\n", i, cpu->memory[i]);
        }
    }

    if (read != size)
    {
        printf("Failed to read entire program\n");
        return -1;
    }

    printf("Loaded %ld bytes into memory\n", size);
    return 0;
}