#ifndef STACK_H_
#define STACK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system/stacktrace.h>

typedef struct {
    size_t capacity;
    size_t size;
    char *bottom;
} Stack;

static inline
void destroy_stack(Stack stack)
{
    free(stack.bottom);
}

static inline
void stack_grow(Stack *stack, size_t new_capacity)
{
    trace_assert(stack);
    trace_assert(stack->capacity < new_capacity);

    stack->bottom = realloc(stack->bottom, new_capacity);
    stack->capacity = new_capacity;
}

static inline
void stack_push(Stack *stack, const void *element, size_t element_size)
{
    trace_assert(stack);
    trace_assert(element);
    trace_assert(element_size > 0);

    size_t frame_size = element_size + sizeof(element_size);

    if (frame_size >= (stack->capacity - stack->size)) {
        stack_grow(stack, stack->capacity * 2 + frame_size);
    }

    trace_assert(stack->bottom);

    memcpy(stack->bottom + stack->size, element, element_size);
    stack->size += element_size;
    memcpy(stack->bottom + stack->size, &element_size, sizeof(element_size));
    stack->size += sizeof(element_size);
}

static inline
size_t stack_top_size(const Stack *stack)
{
    trace_assert(stack);
    trace_assert(stack->size > 0);
    trace_assert(stack->bottom);
    return *(size_t *)(stack->bottom + stack->size - sizeof(size_t));
}

static inline
void *stack_top_element(const Stack *stack)
{
    trace_assert(stack);
    trace_assert(stack->size > 0);
    trace_assert(stack->bottom);
    size_t element_size = stack_top_size(stack);
    return stack->bottom + stack->size - element_size - sizeof(size_t);
}

static inline
void stack_pop(Stack *stack)
{
    trace_assert(stack);
    trace_assert(stack->size > 0);
    size_t element_size = stack_top_size(stack);
    stack->size -= element_size + sizeof(size_t);
}

static inline
int stack_empty(Stack *stack)
{
    trace_assert(stack);
    return stack->size > 0;
}

#endif  // STACK_H_
