#include "cache.h"
#include <string.h>
#include <stdio.h>

static inline uint8_t get_cache_index(uint16_t address)
{
    return (address / CACHE_LINE_SIZE) & (NUM_CACHE_LINES - 1);
}

static inline uint8_t get_cache_tag(uint16_t address)
{
    return address >> CACHE_TAG_BITS;
}

static inline uint8_t get_cache_offset(uint16_t address)
{
    return address & (CACHE_LINE_SIZE - 1);
}

void init_cache(InstructionCache *cache)
{
    memset(cache, 0, sizeof(InstructionCache));
}

static void fill_cache_line(InstructionCache *cache, const uint8_t *memory,
                            uint16_t address)
{
    uint8_t index = get_cache_index(address);
    uint8_t tag = get_cache_tag(address);
    uint16_t base_addr = (address / CACHE_LINE_SIZE) * CACHE_LINE_SIZE;

    CacheLine *line = &cache->lines[index];
    line->valid = true;
    line->tag = tag;

    // Copy data from memory to cache line
    memcpy(line->data, &memory[base_addr], CACHE_LINE_SIZE);
}

uint8_t cache_fetch_byte(InstructionCache *cache, const uint8_t *memory,
                         uint16_t address)
{
    uint8_t index = get_cache_index(address);
    uint8_t tag = get_cache_tag(address);
    uint8_t offset = get_cache_offset(address);

    CacheLine *line = &cache->lines[index];

    // Cache miss
    if (!line->valid || line->tag != tag)
    {
        cache->misses++;
        fill_cache_line(cache, memory, address);
        return line->data[offset];
    }

    // Cache hit
    cache->hits++;
    return line->data[offset];
}

void print_cache_stats(const InstructionCache *cache)
{
    uint32_t total_accesses = cache->hits + cache->misses;
    float hit_rate = total_accesses > 0 ? (float)cache->hits / total_accesses * 100.0 : 0.0;

    printf("\nCache Statistics:\n");
    printf("Total accesses: %u\n", total_accesses);
    printf("Cache hits: %u\n", cache->hits);
    printf("Cache misses: %u\n", cache->misses);
    printf("Hit rate: %.2f%%\n", hit_rate);
}