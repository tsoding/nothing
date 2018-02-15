#include <assert.h>
#include <SDL2/SDL.h>

#include "./lt.h"
#include "./error.h"
#include "./dying_rect.h"

struct dying_rect_t
{
    lt_t *lt;

    vec_t position;
    vec_t size;
    color_t color;
    Uint32 duration;
    Uint32 time_passed;
};

dying_rect_t *create_dying_rect(rect_t rect,
                                color_t color,
                                Uint32 duration)
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

    return dying_rect;
}

void destroy_dying_rect(dying_rect_t *dying_rect)
{
    assert(dying_rect);
    RETURN_LT0(dying_rect->lt);
}

/* TODO(#109): Dying Rect animation is too boring */
int dying_rect_render(const dying_rect_t *dying_rect,
                      SDL_Renderer *renderer,
                      const camera_t *camera)
{
    assert(dying_rect);
    assert(renderer);
    assert(camera);

    const float scale = 1.0f - (float) dying_rect->time_passed / (float) dying_rect->duration;
    const vec_t center = vec_sum(
        dying_rect->position,
        vec_scala_mult(
            dying_rect->size,
            0.5f));
    const vec_t scaled_size =
        vec_scala_mult(dying_rect->size, scale);
    const vec_t scaled_position =
        vec_sum(center, vec_scala_mult(scaled_size, -0.5f));

    return camera_fill_rect(
        camera,
        renderer,
        rect_from_vecs(scaled_position, scaled_size),
        dying_rect->color);
}

int dying_rect_update(dying_rect_t *dying_rect,
                      Uint32 delta_time)
{
    assert(dying_rect);
    assert(delta_time > 0);

    if (!dying_rect_is_dead(dying_rect)) {
        dying_rect->time_passed = dying_rect->time_passed + delta_time;
    }

    return 0;
}

int dying_rect_is_dead(const dying_rect_t *dying_rect)
{
    assert(dying_rect);
    return dying_rect->time_passed >= dying_rect->duration;
}
