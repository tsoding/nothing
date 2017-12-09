#include <SDL2/SDL.h>

#include <math.h>
#include <assert.h>

#include "./camera.h"

struct camera_t {
    point_t position;
};

camera_t *create_camera(point_t position)
{
    camera_t *camera = malloc(sizeof(camera_t));

    if (camera == NULL) {
        return NULL;
    }

    camera->position = position;

    return camera;
}

void destroy_camera(camera_t *camera)
{
    assert(camera);

    free(camera);
}

int camera_fill_rect(const camera_t *camera,
                     SDL_Renderer *render,
                     const rect_t *rect)
{
    assert(camera);
    assert(render);
    assert(rect);

    SDL_Rect view_port;

    SDL_RenderGetViewport(render, &view_port);

    SDL_Rect sdl_rect;

    sdl_rect.x = (int) roundf(rect->x - camera->position.x + (float) view_port.w * 0.5f);
    sdl_rect.y = (int) roundf(rect->y - camera->position.y + (float) view_port.h * 0.5f);
    sdl_rect.w = (int) roundf(rect->w);
    sdl_rect.h = (int) roundf(rect->h);

    return SDL_RenderFillRect(render, &sdl_rect);
}

void camera_center_at(camera_t *camera, point_t position)
{
    assert(camera);
    camera->position = position;
}
