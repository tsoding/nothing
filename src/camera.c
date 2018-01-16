#include <SDL2/SDL.h>

#include <math.h>
#include <assert.h>

#include "./camera.h"
#include "./error.h"

struct camera_t {
    int debug_mode;
    point_t position;
};

camera_t *create_camera(point_t position)
{
    camera_t *camera = malloc(sizeof(camera_t));

    if (camera == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        return NULL;
    }

    camera->position = position;
    camera->debug_mode = 0;

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

    const float khooy_x = 16.0f;
    const float khooy_y = (float) view_port.h / ((float) view_port.w / khooy_x);

    const float scale_x = (float) view_port.w / (khooy_x * 50.0f);
    const float scale_y = (float) view_port.h / (khooy_y * 50.0f);

    SDL_Rect sdl_rect;

    /* TODO(#55): make the scale depend on the view port.
     *
     * So the game looks the same regardless of the size of the window */
    sdl_rect.x = (int) roundf((rect->x - camera->position.x) * scale_x + (float) view_port.w * 0.5f);
    sdl_rect.y = (int) roundf((rect->y - camera->position.y) * scale_y + (float) view_port.h * 0.5f);
    sdl_rect.w = (int) roundf(rect->w * scale_x);
    sdl_rect.h = (int) roundf(rect->h * scale_y);

    if (camera->debug_mode) {
        if (SDL_RenderDrawRect(render, &sdl_rect) < 0) {
            throw_error(ERROR_TYPE_SDL2);
            return -1;
        }
    } else {
        if (SDL_RenderFillRect(render, &sdl_rect) < 0) {
            throw_error(ERROR_TYPE_SDL2);
            return -1;
        }
    }

    return 0;
}

void camera_center_at(camera_t *camera, point_t position)
{
    assert(camera);
    camera->position = position;
}

void camera_toggle_debug_mode(camera_t *camera)
{
    assert(camera);
    camera->debug_mode = !camera->debug_mode;
}
