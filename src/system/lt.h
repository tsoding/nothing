#ifndef LT_H_
#define LT_H_

#include <assert.h>
#include <string.h>

#define LT_INITIAL_CAPACITY 2

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

static inline void destroy_lt(Lt lt)
{
    for (size_t i = lt.size; i > 0; --i) {
        if (lt.slots[i - 1].res) {
            lt.slots[i - 1].dtor(lt.slots[i - 1].res);
        }
    }

    free(lt.slots);
}

#define PUSH_LT(lt, res, dtor)                  \
    lt_push(&lt, (void*)res, (Dtor)dtor)

static inline void *lt_push(Lt lt, void *res, Dtor dtor)
{
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

#define RETURN_LT0(lt) (destroy_lt(lt), return)

#define RESET_LT(lt, old_res, new_res)          \
    lt_reset(&lt, (void*) old_res, (void*) new_res)

static inline void *lt_reset(Lt lt, void *old_res, void *new_res)
{
    assert(old_res != new_res);

    for (size_t i = 0; i < lt->size; ++i) {
        if (lt->slots[i].res == old_res) {
            lt->slots[i].dtor(old_res);
            lt->slots[i].res = new_res;
            return new_res;
        }
    }
    return NULL;
}


#define REPLACE_LT(lt, old_res, new_res)        \
    lt_replace(&lt, (void *)old_res, (void*)new_res)

static inline void *lt_replace(Lt lt, void *old_res, void *new_res)
{
    for (size_t i = 0; i < lt->size; ++i) {
        if (lt->slots[i].res == old_res) {
            lt->slots[i].res = new_res;
            return new_res;
        }
    }

    return NULL;
}

#define RELEASE_LT(lt, res)                     \
    lt_release(&lt, (void*)res)

static inline void *lt_release(Lt lt, void *res)
{
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

    return NULL;
}

#endif  // LT_H_
