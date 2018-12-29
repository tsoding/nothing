#include <SDL2/SDL.h>
#include "system/stacktrace.h"

#include "dying_rect.h"
#include "math/rand.h"
#include "system/lt.h"
#include "system/nth_alloc.h"

#define DYING_RECT_PIECE_COUNT 20
#define DYING_RECT_PIECE_SIZE 20.0f

typedef struct Piece {
    Point position;
    float angle;
    float angle_velocity;
    Vec direction;
    Triangle body;
} Piece;

struct Dying_rect
{
    Lt *lt;

    Vec position;
    Color color;
    float duration;
    float time_passed;
    Piece *pieces;
};

Dying_rect *create_dying_rect(Color color,
                                float duration)
{
    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Dying_rect *dying_rect = PUSH_LT(lt, nth_alloc(sizeof(Dying_rect)), free);
    if (dying_rect == NULL) {
        RETURN_LT(lt, NULL);
    }

    dying_rect->lt = lt;
    dying_rect->position = vec(0.0f, 0.0f);
    dying_rect->color = color;
    dying_rect->duration = duration;
    dying_rect->time_passed = duration;

    dying_rect->pieces = PUSH_LT(lt, nth_alloc(sizeof(Piece) * DYING_RECT_PIECE_COUNT), free);
    if (dying_rect->pieces == NULL) {
        RETURN_LT(lt, NULL);
    }

    return dying_rect;
}

void destroy_dying_rect(Dying_rect *dying_rect)
{
    trace_assert(dying_rect);
    RETURN_LT0(dying_rect->lt);
}

int dying_rect_render(const Dying_rect *dying_rect,
                      Camera *camera)
{
    trace_assert(dying_rect);
    trace_assert(camera);

    for (size_t i = 0; i < DYING_RECT_PIECE_COUNT; ++i) {
        Color color = dying_rect->color;
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

int dying_rect_update(Dying_rect *dying_rect,
                      float delta_time)
{
    trace_assert(dying_rect);
    trace_assert(delta_time > 0.0f);

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

int dying_rect_is_dead(const Dying_rect *dying_rect)
{
    trace_assert(dying_rect);
    return dying_rect->time_passed >= dying_rect->duration;
}

void dying_rect_start_dying(Dying_rect *dying_rect,
                            Vec position)
{
    dying_rect->position = position;
    dying_rect->time_passed = 0;

    for (size_t i = 0; i < DYING_RECT_PIECE_COUNT; ++i) {
        dying_rect->pieces[i].position = dying_rect->position;
        dying_rect->pieces[i].angle = rand_float(2 * PI);
        dying_rect->pieces[i].angle_velocity = rand_float(8.0f);
        dying_rect->pieces[i].body = random_triangle(DYING_RECT_PIECE_SIZE);
        dying_rect->pieces[i].direction = vec_from_polar(
            rand_float_range(-PI, 0.0f),
            rand_float_range(100.0f, 300.0f));
    }
}
