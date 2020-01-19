#ifndef LT_H_
#define LT_H_

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "system/stacktrace.h"

#define LT_INITIAL_CAPACITY 8

typedef void (*Dtor)(void*);

typedef struct {
    void *res;
    Dtor dtor;
} Slot;

typedef struct {
    Slot *slots_end;
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

    if (lt->slots_end) {
        for (Slot *p = lt->slots_end - 1; p >= lt->slots; --p) {
            if (p->res) {
                p->dtor(p->res);
            }
        }
    }

    if (lt->slots) {
        free(lt->slots);
    }

    free(lt);
}

#define PUSH_LT(lt, res, dtor)                  \
    lt_push(lt, (void*)res, (Dtor)dtor)

static inline void *lt_push(Lt *lt, void *res, Dtor dtor)
{
    trace_assert(lt);
    size_t size = (size_t)(lt->slots_end - lt->slots);
    if (size >= lt->capacity) {
        if (lt->capacity == 0) {
            lt->capacity = LT_INITIAL_CAPACITY;
            lt->slots = calloc(LT_INITIAL_CAPACITY, sizeof(Slot));
            lt->slots_end = lt->slots;
        } else {
            lt->capacity *= 2;
            lt->slots = realloc(lt->slots, lt->capacity * sizeof(Slot));
            lt->slots_end = lt->slots + size;
        }
    }

    lt->slots_end->res = res;
    lt->slots_end->dtor = dtor;
    lt->slots_end++;

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

    for(Slot *p = lt->slots; p < lt->slots_end; ++p) {
        if (p->res == old_res) {
            p->dtor(old_res);
            p->res = new_res;
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
    for(Slot *p = lt->slots; p < lt->slots_end; ++p) {
        if (p->res == old_res) {
            p->res = new_res;
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
    for(Slot *p = lt->slots; p < lt->slots_end; ++p) {
        if (p->res == res) {
            memmove(p, p + 1, (size_t)(lt->slots_end - p - 1) * sizeof(Slot));
            lt->slots_end--;
            return res;
        }
    }

    trace_assert(0 && "Resource was not found");
    return NULL;
}

#endif  // LT_H_
