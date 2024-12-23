#include "tiny_x86.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void print_test_result(const char *test_name, bool passed)
{
    printf("%-40s %s\n", test_name, passed ? "[PASS]" : "[FAIL]");
}

void reset_cpu(CPU *cpu)
{
    init_cpu(cpu);
}

void execute_non_verbose(CPU *cpu)
{
    execute(cpu, false);
}

void execute_verbose(CPU *cpu)
{
    execute(cpu, true);
}

void test_mov_immediate()
{
    CPU cpu;
    reset_cpu(&cpu);

    // Test MOV AL, 0x42
    uint8_t program1[] = {0xB0, 0x42, 0xF4};
    memcpy(cpu.memory, program1, sizeof(program1));
    execute_non_verbose(&cpu);
    print_test_result("MOV AL, immediate", cpu.al == 0x42);

    // Test MOV BH, 0xFF
    reset_cpu(&cpu);
    uint8_t program2[] = {0xB7, 0xFF, 0xF4};
    memcpy(cpu.memory, program2, sizeof(program2));
    execute_non_verbose(&cpu);
    print_test_result("MOV BH, immediate", cpu.bh == 0xFF);
}

void test_mov_register()
{
    CPU cpu;
    reset_cpu(&cpu);

    // Test MOV BL, AL (after setting AL)
    uint8_t program[] = {0xB0, 0x42, // MOV AL, 0x42
                         0x88, 0xC3, // MOV BL, AL
                         0xF4};
    memcpy(cpu.memory, program, sizeof(program));
    execute_non_verbose(&cpu); // Execute MOV AL, 0x42
    execute_non_verbose(&cpu); // Execute MOV BL, AL
    print_test_result("MOV register to register", cpu.bl == 0x42);
}

void test_arithmetic()
{
    CPU cpu;
    reset_cpu(&cpu);

    // Test ADD
    uint8_t add_program[] = {0xB0, 0x05, // MOV AL, 5
                             0xB3, 0x03, // MOV BL, 3
                             0x00, 0xD8, // ADD AL, BL
                             0xF4};
    memcpy(cpu.memory, add_program, sizeof(add_program));
    execute_non_verbose(&cpu); // MOV AL, 5
    execute_non_verbose(&cpu); // MOV BL, 3
    execute_non_verbose(&cpu); // ADD AL, BL
    print_test_result("ADD registers", cpu.al == 8);

    // Test SUB
    reset_cpu(&cpu);
    uint8_t sub_program[] = {0xB0, 0x0A, // MOV AL, 10
                             0x2C, 0x03, // SUB AL, 3
                             0xF4};
    memcpy(cpu.memory, sub_program, sizeof(sub_program));
    execute_non_verbose(&cpu); // MOV AL, 10
    execute_non_verbose(&cpu); // SUB AL, 3
    print_test_result("SUB immediate", cpu.al == 7);
}

void test_inc_dec()
{
    CPU cpu;
    reset_cpu(&cpu);

    // Test INC
    uint8_t inc_program[] = {0xB0, 0x05, // MOV AL, 5
                             0xFE, 0xC0, // INC AL
                             0xF4};
    memcpy(cpu.memory, inc_program, sizeof(inc_program));
    execute_non_verbose(&cpu); // MOV AL, 5
    execute_non_verbose(&cpu); // INC AL
    print_test_result("INC register", cpu.al == 6);

    // Test DEC
    reset_cpu(&cpu);
    uint8_t dec_program[] = {0xB0, 0x05, // MOV AL, 5
                             0xFE, 0xC8, // DEC AL
                             0xF4};
    memcpy(cpu.memory, dec_program, sizeof(dec_program));
    execute_non_verbose(&cpu); // MOV AL, 5
    execute_non_verbose(&cpu); // DEC AL
    print_test_result("DEC register", cpu.al == 4);
}

void test_logic()
{
    CPU cpu;
    reset_cpu(&cpu);

    // Test AND
    uint8_t and_program[] = {0xB0, 0x0F, // MOV AL, 0x0F
                             0xB3, 0x33, // MOV BL, 0x33
                             0x20, 0xD8, // AND AL, BL
                             0xF4};
    memcpy(cpu.memory, and_program, sizeof(and_program));
    execute_non_verbose(&cpu); // MOV AL, 0x0F
    execute_non_verbose(&cpu); // MOV BL, 0x33
    execute_non_verbose(&cpu); // AND AL, BL
    print_test_result("AND registers", cpu.al == (0x0F & 0x33));

    // Test OR
    reset_cpu(&cpu);
    uint8_t or_program[] = {0xB0, 0x0F, // MOV AL, 0x0F
                            0xB3, 0x30, // MOV BL, 0x30
                            0x08, 0xD8, // OR AL, BL
                            0xF4};
    memcpy(cpu.memory, or_program, sizeof(or_program));
    execute_non_verbose(&cpu); // MOV AL, 0x0F
    execute_non_verbose(&cpu); // MOV BL, 0x30
    execute_non_verbose(&cpu); // OR AL, BL
    print_test_result("OR registers", cpu.al == (0x0F | 0x30));
}

void test_shifts()
{
    CPU cpu;
    reset_cpu(&cpu);

    // Test SHL by 1
    uint8_t shl_program[] = {0xB0, 0x02, // MOV AL, 2
                             0xD0, 0xE0, // SHL AL, 1
                             0xF4};
    memcpy(cpu.memory, shl_program, sizeof(shl_program));
    execute_non_verbose(&cpu); // MOV AL, 2
    execute_non_verbose(&cpu); // SHL AL, 1
    print_test_result("SHL by 1", cpu.al == 4);

    // Test SHR by 1
    reset_cpu(&cpu);
    uint8_t shr_program[] = {0xB0, 0x08, // MOV AL, 8
                             0xD0, 0xE8, // SHR AL, 1
                             0xF4};
    memcpy(cpu.memory, shr_program, sizeof(shr_program));
    execute_non_verbose(&cpu); // MOV AL, 8
    execute_non_verbose(&cpu); // SHR AL, 1
    print_test_result("SHR by 1", cpu.al == 4);
}

void test_jumps()
{
    CPU cpu;
    reset_cpu(&cpu);

    // Test JMP
    uint8_t jmp_program[] = {0xEB, 0x02, // JMP +2
                             0xB0, 0x01, // MOV AL, 1 (should be skipped)
                             0xB0, 0x02, // MOV AL, 2 (should be executed)
                             0xF4};
    memcpy(cpu.memory, jmp_program, sizeof(jmp_program));
    execute_non_verbose(&cpu); // JMP +2
    execute_non_verbose(&cpu); // MOV AL, 2
    print_test_result("JMP forward", cpu.al == 2);

    // Test JE (when equal)
    reset_cpu(&cpu);
    uint8_t je_program[] = {0xB0, 0x05, // MOV AL, 5
                            0x3C, 0x05, // CMP AL, 5
                            0x74, 0x02, // JE +2
                            0xB0, 0x01, // MOV AL, 1 (should be skipped)
                            0xB0, 0x02, // MOV AL, 2 (should be executed)
                            0xF4};
    memcpy(cpu.memory, je_program, sizeof(je_program));
    execute_non_verbose(&cpu); // MOV AL, 5
    execute_non_verbose(&cpu); // CMP AL, 5
    execute_non_verbose(&cpu); // JE +2
    execute_non_verbose(&cpu); // MOV AL, 2
    print_test_result("JE when equal", cpu.al == 2);
}

void test_stack()
{
    CPU cpu;
    reset_cpu(&cpu);

    // Test PUSH/POP AX
    uint8_t stack_program[] = {0xB0, 0x12, // MOV AL, 0x12
                               0xB4, 0x34, // MOV AH, 0x34
                               0x50,       // PUSH AX
                               0xB0, 0x00, // MOV AL, 0
                               0xB4, 0x00, // MOV AH, 0
                               0x58,       // POP AX
                               0xF4};
    memcpy(cpu.memory, stack_program, sizeof(stack_program));
    execute_non_verbose(&cpu); // MOV AL, 0x12
    execute_non_verbose(&cpu); // MOV AH, 0x34
    execute_non_verbose(&cpu); // PUSH AX
    execute_non_verbose(&cpu); // MOV AL, 0
    execute_non_verbose(&cpu); // MOV AH, 0
    execute_non_verbose(&cpu); // POP AX
    print_test_result("PUSH/POP AX", cpu.al == 0x12 && cpu.ah == 0x34);
}

void test_flags()
{
    CPU cpu;
    reset_cpu(&cpu);

    // Test Zero Flag
    uint8_t zf_program[] = {0xB0, 0x05, // MOV AL, 5
                            0x2C, 0x05, // SUB AL, 5
                            0xF4};
    memcpy(cpu.memory, zf_program, sizeof(zf_program));
    execute_non_verbose(&cpu); // MOV AL, 5
    execute_non_verbose(&cpu); // SUB AL, 5
    print_test_result("Zero Flag set on zero result", (cpu.flags & FLAG_ZERO) != 0);

    // Test Sign Flag
    reset_cpu(&cpu);
    uint8_t sf_program[] = {0xB0, 0x05, // MOV AL, 5
                            0x2C, 0x06, // SUB AL, 6
                            0xF4};
    memcpy(cpu.memory, sf_program, sizeof(sf_program));
    execute_non_verbose(&cpu); // MOV AL, 5
    execute_non_verbose(&cpu); // SUB AL, 6
    print_test_result("Sign Flag set on negative result", (cpu.flags & FLAG_SIGN) != 0);
}

int main()
{
    printf("Starting x86 Emulator Tests\n");
    printf("=====================================\n");

    test_mov_immediate();
    test_mov_register();
    test_arithmetic();
    test_inc_dec();
    test_logic();
    test_shifts();
    test_jumps();
    test_stack();
    test_flags();

    printf("=====================================\n");
    printf("Test suite completed\n");
    return 0;
}