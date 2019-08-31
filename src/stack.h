#ifndef STACK_H_
#define STACK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system/stacktrace.h>

typedef struct {
    size_t capacity;
    size_t size;
    void *bottom;
} Stack;

void stack_grow(Stack *stack, size_t new_capacity)
{
    trace_assert(stack);
    trace_assert(stack->capacity < new_capacity);
    trace_assert(stack->bottom);

    stack->bottom = realloc(stack->bottom, new_capacity);
    stack->capacity = new_capacity;
}

void stack_push(Stack *stack, const void *element, size_t element_size)
{
    trace_assert(stack);
    trace_assert(element);
    trace_assert(element_size > 0);

    size_t frame_size = element_size + sizeof(element_size);

    if (frame_size >= (stack->capacity - stack->size)) {
        stack_grow(stack, stack->capacity * 2 + frame_size);
    }

    memcpy(stack->bottom + stack->size, element, element_size);
    stack->size += element_size;
    memcpy(stack->bottom + stack->size, &element_size, sizeof(element_size));
    stack->size += sizeof(element_size);
}

size_t stack_top_size(const Stack *stack)
{
    trace_assert(stack);
    trace_assert(stack->size > 0);
    return *(size_t *)(stack->bottom + stack->size - sizeof(size_t));
}

void *stack_top_frame(const Stack *stack)
{
    trace_assert(stack);
    trace_assert(stack->size > 0);
    size_t element_size = stack_top_size(stack);
    return stack->bottom + stack->size - element_size - sizeof(size_t);
}

void stack_pop(Stack *stack)
{
    trace_assert(stack);
    trace_assert(stack->size > 0);
    size_t element_size = stack_top_size(stack);
    stack->size -= element_size + sizeof(size_t);
}

#endif  // STACK_H_
