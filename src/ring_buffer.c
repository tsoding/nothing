#include "ring_buffer.h"
#include "system/stacktrace.h"

void ring_buffer_push(RingBuffer *buffer,
                      void *element)
{
    trace_assert(buffer);
    trace_assert(element);

    size_t i = (buffer->begin + buffer->count) % buffer->capacity;

    if (buffer->count < buffer->capacity) {
        memcpy(
            buffer->data + i * buffer->element_size,
            element,
            buffer->element_size);
        buffer->count += 1;
    } else {
        memcpy(
            buffer->data + i * buffer->element_size,
            element,
            buffer->element_size);
        buffer->begin = (buffer->begin + 1) % buffer->capacity;
    }
}

int ring_buffer_pop(RingBuffer *buffer)
{
    trace_assert(buffer);

    if (buffer->count == 0) return 0;
    buffer->count--;

    return 1;
}

void *ring_buffer_top(RingBuffer *buffer)
{
    trace_assert(buffer);
    if (buffer->count == 0) return NULL;
    size_t i = (buffer->begin + buffer->count - 1) % buffer->capacity;
    return buffer->data + i * buffer->element_size;
}
