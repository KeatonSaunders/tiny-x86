#ifndef CPU_CACHE_H
#define CPU_CACHE_H

#include <stdint.h>
#include <stdbool.h>

#define CACHE_SIZE 256
#define CACHE_LINE_SIZE 8
#define NUM_CACHE_LINES (CACHE_SIZE / CACHE_LINE_SIZE)
#define CACHE_TAG_BITS 8

typedef struct
{
    bool valid;
    uint8_t tag;
    uint8_t data[CACHE_LINE_SIZE];
} CacheLine;

typedef struct
{
    CacheLine lines[NUM_CACHE_LINES];
    uint32_t hits;
    uint32_t misses;
} InstructionCache;

void init_cache(InstructionCache *cache);
uint8_t cache_fetch_byte(InstructionCache *cache, const uint8_t *memory, uint16_t address);
void print_cache_stats(const InstructionCache *cache);

#endif