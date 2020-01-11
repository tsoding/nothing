#ifndef MEMORY_H_
#define MEMORY_H_

#include <assert.h>
#include <stdint.h>

#define KILO 1024L
#define MEGA (1024L * KILO)
#define GIGA (1024L * MEGA)

typedef struct {
    size_t capacity;
    size_t size;
    uint8_t *buffer;
} Memory;

static inline
void *memory_alloc(Memory *memory, size_t size)
{
    assert(memory);
    assert(memory->size + size <= memory->capacity);

    void *result = memory->buffer + memory->size;
    memory->size += size;

    return result;
}

static inline
void memory_clean(Memory *memory)
{
    assert(memory);
    memory->size = 0;
}

#endif  // MEMORY_H_
