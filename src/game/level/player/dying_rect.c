#include <SDL2/SDL.h>
#include <assert.h>

#include "dying_rect.h"
#include "math/rand.h"
#include "system/error.h"
#include "system/lt.h"

#define DYING_RECT_PIECE_COUNT 20
#define DYING_RECT_PIECE_SIZE 20.0f

typedef struct piece_t {
    point_t position;
    float angle;
    float angle_velocity;
    vec_t direction;
    triangle_t body;
} piece_t;

struct dying_rect_t
{
    lt_t *lt;

    vec_t position;
    vec_t size;
    color_t color;
    float duration;
    float time_passed;
    piece_t *pieces;
};

dying_rect_t *create_dying_rect(rect_t rect,
                                color_t color,
                                float duration)
{
    lt_t *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    dying_rect_t *dying_rect = PUSH_LT(lt, malloc(sizeof(dying_rect_t)), free);
    if (dying_rect == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    dying_rect->lt = lt;
    dying_rect->position = vec(rect.x, rect.y);
    dying_rect->size = vec(rect.w, rect.h);
    dying_rect->color = color;
    dying_rect->duration = duration;
    dying_rect->time_passed = 0;

    dying_rect->pieces = PUSH_LT(lt, malloc(sizeof(piece_t) * DYING_RECT_PIECE_COUNT), free);
    if (dying_rect->pieces == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    for (size_t i = 0; i < DYING_RECT_PIECE_COUNT; ++i) {
        dying_rect->pieces[i].position = dying_rect->position;
        dying_rect->pieces[i].angle = rand_float(2 * PI);
        dying_rect->pieces[i].angle_velocity = rand_float(8.0f);
        dying_rect->pieces[i].body = random_triangle(DYING_RECT_PIECE_SIZE);
        dying_rect->pieces[i].direction = vec_from_polar(
            rand_float_range(-PI, 0.0f),
            rand_float_range(100.0f, 300.0f));
    }

    return dying_rect;
}

void destroy_dying_rect(dying_rect_t *dying_rect)
{
    assert(dying_rect);
    RETURN_LT0(dying_rect->lt);
}

int dying_rect_render(const dying_rect_t *dying_rect,
                      camera_t *camera)
{
    assert(dying_rect);
    assert(camera);

    for (size_t i = 0; i < DYING_RECT_PIECE_COUNT; ++i) {
        color_t color = dying_rect->color;
        color.a = fminf(1.0f, 4.0f - (float) dying_rect->time_passed / (float) dying_rect->duration * 4.0f);

        if (camera_fill_triangle(
                camera,
                triangle_mat3x3_product(
                    dying_rect->pieces[i].body,
                    mat3x3_product(
                        trans_mat(dying_rect->pieces[i].position.x,
                                  dying_rect->pieces[i].position.y),
                        rot_mat(dying_rect->pieces[i].angle))),
                color) < 0) {
            return -1;
        }
    }

    return 0;
}

int dying_rect_update(dying_rect_t *dying_rect,
                      float delta_time)
{
    assert(dying_rect);
    assert(delta_time > 0.0f);

    if (dying_rect_is_dead(dying_rect)) {
        return 0;
    }

    dying_rect->time_passed = dying_rect->time_passed + delta_time;

    for (size_t i = 0; i < DYING_RECT_PIECE_COUNT; ++i) {
        vec_add(
            &dying_rect->pieces[i].position,
            vec_scala_mult(
                dying_rect->pieces[i].direction,
                delta_time));
        dying_rect->pieces[i].angle = fmodf(
            dying_rect->pieces[i].angle + dying_rect->pieces[i].angle_velocity * delta_time,
            2.0f * PI);
    }

    return 0;
}

int dying_rect_is_dead(const dying_rect_t *dying_rect)
{
    assert(dying_rect);
    return dying_rect->time_passed >= dying_rect->duration;
}
