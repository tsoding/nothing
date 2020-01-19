#include <SDL.h>
#include "system/stacktrace.h"

#include "explosion.h"
#include "math/rand.h"
#include "math/mat3x3.h"
#include "system/lt.h"
#include "system/nth_alloc.h"

#define EXPLOSION_PIECE_COUNT 20
#define EXPLOSION_PIECE_SIZE 20.0f

typedef struct Piece {
    Vec2f position;
    float angle;
    float angle_velocity;
    Vec2f direction;
    Triangle body;
} Piece;

struct Explosion
{
    Lt *lt;

    Vec2f position;
    Color color;
    float duration;
    float time_passed;
    Piece *pieces;
};

Explosion *create_explosion(Color color,
                            float duration)
{
    Lt *lt = create_lt();

    Explosion *explosion = PUSH_LT(lt, nth_calloc(1, sizeof(Explosion)), free);
    if (explosion == NULL) {
        RETURN_LT(lt, NULL);
    }

    explosion->lt = lt;
    explosion->position = vec(0.0f, 0.0f);
    explosion->color = color;
    explosion->duration = duration;
    explosion->time_passed = duration;

    explosion->pieces = PUSH_LT(lt, nth_calloc(1, sizeof(Piece) * EXPLOSION_PIECE_COUNT), free);
    if (explosion->pieces == NULL) {
        RETURN_LT(lt, NULL);
    }

    return explosion;
}

void destroy_explosion(Explosion *explosion)
{
    trace_assert(explosion);
    RETURN_LT0(explosion->lt);
}

int explosion_render(const Explosion *explosion,
                     const Camera *camera)
{
    trace_assert(explosion);
    trace_assert(camera);

    for (size_t i = 0; i < EXPLOSION_PIECE_COUNT; ++i) {
        Color color = explosion->color;
        color.a = fminf(1.0f, 4.0f - (float) explosion->time_passed / (float) explosion->duration * 4.0f);

        if (camera_fill_triangle(
                camera,
                triangle_mat3x3_product(
                    explosion->pieces[i].body,
                    mat3x3_product(
                        trans_mat(explosion->pieces[i].position.x,
                                  explosion->pieces[i].position.y),
                        rot_mat(explosion->pieces[i].angle))),
                color) < 0) {
            return -1;
        }
    }

    return 0;
}

int explosion_update(Explosion *explosion,
                     float delta_time)
{
    trace_assert(explosion);
    trace_assert(delta_time > 0.0f);

    if (explosion_is_done(explosion)) {
        return 0;
    }

    explosion->time_passed = explosion->time_passed + delta_time;

    for (size_t i = 0; i < EXPLOSION_PIECE_COUNT; ++i) {
        vec_add(
            &explosion->pieces[i].position,
            vec_scala_mult(
                explosion->pieces[i].direction,
                delta_time));
        explosion->pieces[i].angle = fmodf(
            explosion->pieces[i].angle + explosion->pieces[i].angle_velocity * delta_time,
            2.0f * PI);
    }

    return 0;
}

int explosion_is_done(const Explosion *explosion)
{
    trace_assert(explosion);
    return explosion->time_passed >= explosion->duration;
}

void explosion_start(Explosion *explosion,
                     Vec2f position)
{
    explosion->position = position;
    explosion->time_passed = 0;

    for (size_t i = 0; i < EXPLOSION_PIECE_COUNT; ++i) {
        explosion->pieces[i].position = explosion->position;
        explosion->pieces[i].angle = rand_float(2 * PI);
        explosion->pieces[i].angle_velocity = rand_float(8.0f);
        explosion->pieces[i].body = random_triangle(EXPLOSION_PIECE_SIZE);
        explosion->pieces[i].direction = vec_from_polar(
            rand_float_range(-PI, 0.0f),
            rand_float_range(100.0f, 300.0f));
    }
}
