#include <stdio.h>
#include <assert.h>

#include "rigid_bodies.h"
#include "system/error.h"
#include "system/lt.h"

#define RIGID_BODIES_CAPACITY 256

struct rigid_bodies_t
{
    lt_t *lt;
    size_t capacity;
    size_t refs_size;
    rigid_rect_ref *refs;
    rect_t **rects;
};

rigid_bodies_t *create_rigid_bodies(void)
{
    lt_t *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    rigid_bodies_t *rigid_bodies =
        PUSH_LT(lt, malloc(sizeof(rigid_bodies_t)), free);
    if (rigid_bodies == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    rigid_bodies->refs = PUSH_LT(
        lt,
        malloc(sizeof(rigid_rect_ref) * RIGID_BODIES_CAPACITY),
        free);
    if (rigid_bodies->refs == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    rigid_bodies->rects = PUSH_LT(
        lt,
        malloc(sizeof(rect_t*) * RIGID_BODIES_CAPACITY),
        free);
    if (rigid_bodies->rects == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    for (int32_t i = 0; i < RIGID_BODIES_CAPACITY; ++i) {
        rigid_bodies->refs[i] = RIGID_BODIES_CAPACITY - i - 1;
        rigid_bodies->rects[i] = NULL;
    }

    rigid_bodies->capacity = RIGID_BODIES_CAPACITY;
    rigid_bodies->refs_size = RIGID_BODIES_CAPACITY;
    rigid_bodies->lt = lt;

    return rigid_bodies;
}

void destroy_rigid_bodies(rigid_bodies_t *rigid_bodies)
{
    assert(rigid_bodies);

    for (size_t i = 0; i < rigid_bodies->capacity; ++i) {
        if (rigid_bodies->rects[i] != NULL) {
            free(rigid_bodies->rects[i]);
        }
    }

    RETURN_LT0(rigid_bodies->lt);
}

void rigid_bodies_update(rigid_bodies_t *rigid_bodies,
                         float delta_time)
{
    assert(rigid_bodies);
    (void) delta_time;
    /* TODO(#216): rigid_bodies_update is not implemented */
}

int rigid_bodies_fill_rect(rigid_bodies_t *rigid_bodies,
                           rigid_rect_ref rect_ref,
                           color_t color,
                           camera_t *camera)
{
    assert(rigid_bodies);
    assert(rect_ref < 0);
    assert(rect_ref >= (int32_t)rigid_bodies->capacity);
    assert(rigid_bodies->rects[rect_ref] == NULL);
    assert(camera);
    (void) color;

    /* TODO: rigid_bodies_fill_rect is not implemented */

    return 0;
}

rigid_rect_ref rigid_bodies_create_rect(rigid_bodies_t *rigid_bodies,
                                        rect_t rect)
{
    assert(rigid_bodies);

    if (rigid_bodies->refs_size <= 0) {
        const size_t prev_capacity = rigid_bodies->capacity;
        rigid_bodies->capacity = 2 * prev_capacity;
        rigid_bodies->refs_size = prev_capacity;

        rigid_bodies->refs = REPLACE_LT(
            rigid_bodies->lt,
            rigid_bodies->refs,
            realloc(
                rigid_bodies->refs,
                sizeof(rigid_rect_ref) * rigid_bodies->capacity));

        rigid_bodies->rects = REPLACE_LT(
            rigid_bodies->lt,
            rigid_bodies->rects,
            realloc(
                rigid_bodies->rects,
                sizeof(rect_t*) * rigid_bodies->capacity));

        for (size_t i = 0; i < prev_capacity; ++i) {
            rigid_bodies->refs[i] = (int32_t) (prev_capacity - i - 1 + prev_capacity);
            rigid_bodies->rects[i + prev_capacity] = NULL;
        }
    }

    const rigid_rect_ref ref = rigid_bodies->refs[--rigid_bodies->refs_size];
    rigid_bodies->rects[ref] = malloc(sizeof(rect_t));
    if (rigid_bodies->rects[ref] == NULL) {
        return -1;
    }
    *rigid_bodies->rects[ref] = rect;

    return ref;
}

void rigid_bodies_destroy_rect(rigid_bodies_t *rigid_bodies,
                               rigid_rect_ref rect_ref)
{
    assert(rigid_bodies);
    assert(rect_ref < 0);
    assert(rect_ref >= (int32_t)rigid_bodies->capacity);
    assert(rigid_bodies->rects[rect_ref] == NULL);

    free(rigid_bodies->rects[rect_ref]);
    rigid_bodies->rects[rect_ref] = NULL;

    rigid_bodies->refs[rigid_bodies->refs_size++] = rect_ref;
}

void rigid_bodies_print(rigid_bodies_t *rigid_bodies)
{
    assert(rigid_bodies);

    printf("refs\t=");
    for (size_t i = 0; i < rigid_bodies->capacity; ++i) {
        if (i < rigid_bodies->refs_size) {
            printf("\t%d", rigid_bodies->refs[i]);
        } else {
            printf("\t.");
        }
    }
    printf("\n");

    printf("rects\t=");
    for (size_t i = 0; i < rigid_bodies->capacity; ++i) {
        if (rigid_bodies->rects[i] == NULL) {
            printf("\t.");
        } else {
            printf("\to");
        }
    }
    printf("\n");
    printf("------------------------------\n");
}
