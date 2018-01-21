#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "./platforms.h"
#include "./error.h"
#include "./lt.h"

struct platforms_t {
    lt_t *lt;

    rect_t *rects;
    size_t rects_size;
};

platforms_t *create_platforms(const rect_t *rects, size_t rects_size)
{
    assert(rects);
    assert(rects_size > 0);

    lt_t *const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    platforms_t *platforms = PUSH_LT(lt, malloc(sizeof(platforms_t)), free);
    if (platforms == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    platforms->rects = PUSH_LT(lt, malloc(sizeof(rect_t) * rects_size), free);
    if (platforms->rects == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    platforms->rects = memcpy(platforms->rects, rects, sizeof(rect_t) * rects_size);
    platforms->rects_size = rects_size;
    platforms->lt = lt;

    return platforms;
}

platforms_t *create_platforms_from_stream(FILE *stream)
{
    assert(stream);

    lt_t *const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    platforms_t *platforms = PUSH_LT(lt, malloc(sizeof(platforms_t)), free);
    if (platforms == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    platforms->rects_size = 0;
    if (fscanf(stream, "%lu", &platforms->rects_size) == EOF) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    platforms->rects = PUSH_LT(lt, malloc(sizeof(rect_t) * platforms->rects_size), free);
    if (platforms->rects == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    for (size_t i = 0; i < platforms->rects_size; ++i) {
        if (fscanf(stream, "%f%f%f%f\n",
                   &platforms->rects[i].x, &platforms->rects[i].y,
                   &platforms->rects[i].w, &platforms->rects[i].h) < 0) {
            throw_error(ERROR_TYPE_LIBC);
            RETURN_LT(lt, NULL);
        }
    }

    platforms->lt = lt;

    return platforms;
}

platforms_t *create_platforms_from_file(const char *filename)
{
    assert(filename);

    FILE *platforms_file = fopen(filename, "r");
    if (platforms_file == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        return NULL;
    }

    platforms_t *platforms = create_platforms_from_stream(platforms_file);
    if (platforms != NULL) {
        fclose(platforms_file);
        return NULL;
    }

    fclose(platforms_file);
    return platforms;
}

void destroy_platforms(platforms_t *platforms)
{
    assert(platforms);
    RETURN_LT0(platforms->lt);
}

int platforms_save_to_file(const platforms_t *platforms,
                           const char *filename)
{
    assert(platforms);
    assert(filename);

    lt_t *const lt = create_lt();
    if (lt == NULL) {
        return -1;
    }

    FILE *platforms_file = PUSH_LT(lt, fopen(filename, "w"), fclose);

    if (platforms_file == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, -1);
    }

    for (size_t i = 0; i < platforms->rects_size; ++i) {
        if (fprintf(platforms_file, "%f %f %f %f\n",
                    platforms->rects[i].x, platforms->rects[i].y,
                    platforms->rects[i].w, platforms->rects[i].h) < 0) {
            throw_error(ERROR_TYPE_LIBC);
            RETURN_LT(lt, -1);
        }
    }

    RETURN_LT(lt, 0);
}

int platforms_render(const platforms_t *platforms,
                     SDL_Renderer *renderer,
                     const camera_t *camera)
{
    if (SDL_SetRenderDrawColor(renderer, 50, 50, 55, 255) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    for (size_t i = 0; i < platforms->rects_size; ++i) {
        if (camera_fill_rect(camera, renderer, &platforms->rects[i]) < 0) {
            throw_error(ERROR_TYPE_SDL2);
            return -1;
        }
    }

    return 0;
}

void platforms_rect_object_collide(const platforms_t *platforms,
                                   rect_t object,
                                   int sides[RECT_SIDE_N])
{
    assert(platforms);

    memset(sides, 0, sizeof(int) * RECT_SIDE_N);

    for (size_t i = 0; i < platforms->rects_size; ++i) {
        rect_object_impact(&object, &platforms->rects[i], sides);
    }
}
