#ifndef LT_H_
#define LT_H_

#include <assert.h>
#include <string.h>
#include "system/stacktrace.h"

#define LT_INITIAL_CAPACITY 8

typedef void (*Dtor)(void*);

typedef struct {
    void *res;
    Dtor dtor;
} Slot;

typedef struct {
    size_t size;
    size_t capacity;
    Slot *slots;
} Lt;

static inline Lt *create_lt(void)
{
    return calloc(1, sizeof(Lt));
}

static inline void destroy_lt(Lt *lt)
{
    trace_assert(lt);

    for (size_t i = lt->size; i > 0; --i) {
        if (lt->slots[i - 1].res) {
            lt->slots[i - 1].dtor(lt->slots[i - 1].res);
        }
    }

    free(lt->slots);
    free(lt);
}

#define PUSH_LT(lt, res, dtor)                  \
    lt_push(lt, (void*)res, (Dtor)dtor)

static inline void *lt_push(Lt *lt, void *res, Dtor dtor)
{
    trace_assert(lt);

    if (lt->size >= lt->capacity) {
        if (lt->capacity == 0) {
            lt->capacity = LT_INITIAL_CAPACITY;
            lt->slots = calloc(LT_INITIAL_CAPACITY, sizeof(Slot));
        } else {
            lt->capacity *= 2;
            lt->slots = realloc(lt->slots, lt->capacity * sizeof(Slot));
        }
    }

    lt->slots[lt->size].res = res;
    lt->slots[lt->size].dtor = dtor;
    lt->size++;

    return res;
}

#define RETURN_LT(lt, result)                   \
    return (destroy_lt(lt), result)

#define RETURN_LT0(lt)                          \
    do {                                        \
        destroy_lt(lt);                         \
        return;                                 \
    } while (0)

#define RESET_LT(lt, old_res, new_res)          \
    lt_reset(lt, (void*) old_res, (void*) new_res)

static inline void *lt_reset(Lt *lt, void *old_res, void *new_res)
{
    trace_assert(lt);
    trace_assert(old_res != new_res);

    for (size_t i = 0; i < lt->size; ++i) {
        if (lt->slots[i].res == old_res) {
            lt->slots[i].dtor(old_res);
            lt->slots[i].res = new_res;
            return new_res;
        }
    }

    trace_assert(0 && "Resource was not found");
    return NULL;
}


#define REPLACE_LT(lt, old_res, new_res)        \
    lt_replace(lt, (void *)old_res, (void*)new_res)

static inline void *lt_replace(Lt *lt, void *old_res, void *new_res)
{
    trace_assert(lt);
    for (size_t i = 0; i < lt->size; ++i) {
        if (lt->slots[i].res == old_res) {
            lt->slots[i].res = new_res;
            return new_res;
        }
    }

    trace_assert(0 && "Resource was not found");
    return NULL;
}

#define RELEASE_LT(lt, res)                     \
    lt_release(lt, (void*)res)

static inline void *lt_release(Lt *lt, void *res)
{
    trace_assert(lt);
    for (size_t i = 0; i < lt->size; ++i) {
        if (lt->slots[i].res == res) {
            memmove(
                &lt->slots[i],
                &lt->slots[i + 1],
                (lt->size - i - 1) * sizeof(Slot));
            lt->size--;
            return res;
        }
    }

    trace_assert(0 && "Resource was not found");
    return NULL;
}

#endif  // LT_H_
