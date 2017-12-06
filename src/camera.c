#include <SDL2/SDL.h>

#include <math.h>
#include <assert.h>

#include "./camera.h"

struct camera_t {
    point_t translation;
};

camera_t *create_camera()
{
    camera_t *camera = malloc(sizeof(camera_t));

    if (camera == NULL) {
        return NULL;
    }

    camera->translation.x = 0;
    camera->translation.y = 0;

    return camera;
}

void destroy_camera(camera_t *camera)
{
    assert(camera);

    free(camera);
}

int camera_fill_rect(const camera_t *camera,
                     SDL_Renderer *render,
                     const struct rect_t *rect)
{
    assert(camera);
    assert(render);
    assert(rect);

    SDL_Rect sdl_rect;

    sdl_rect.x = (int) roundf(rect->x - camera->translation.x);
    sdl_rect.y = (int) roundf(rect->y - camera->translation.y);
    sdl_rect.w = (int) roundf(rect->w);
    sdl_rect.h = (int) roundf(rect->h);

    return SDL_RenderFillRect(render, &sdl_rect);
}

void camera_translate(camera_t *camera, float x, float y)
{
    camera->translation.x = x;
    camera->translation.y = y;
}
