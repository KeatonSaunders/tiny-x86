#include "tiny_x86.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <program.bin>\n", argv[0]);
        return 1;
    }

    CPU cpu;
    init_cpu(&cpu);

    bool verbose = false;
    if (load_program(&cpu, argv[1], verbose) != 0)
    {
        return 1;
    }

    run_cpu(&cpu, verbose);
    return 0;
}