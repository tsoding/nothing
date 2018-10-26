#include <assert.h>

#include "game/camera.h"
#include "game/level/labels.h"
#include "system/error.h"
#include "system/lt.h"
#include "str.h"

#define LABEL_TEXT_MAX_LENGTH 64

struct Labels
{
    Lt *lt;
    size_t count;
    Vec *positions;
    Color *colors;
    char **texts;
    float *states;
    int *visible;
};

Labels *create_labels_from_stream(FILE *stream)
{
    assert(stream);

    Lt *const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Labels * const labels = PUSH_LT(lt, malloc(sizeof(Labels)), free);
    if (labels == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    labels->lt = lt;

    if (fscanf(stream, "%lu", &labels->count) == EOF) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    labels->positions = PUSH_LT(lt, malloc(sizeof(Vec) * labels->count), free);
    if (labels->positions == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    labels->colors = PUSH_LT(lt, malloc(sizeof(Color) * labels->count), free);
    if (labels->colors == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    labels->texts = PUSH_LT(lt, malloc(sizeof(char*) * labels->count), free);
    if (labels->texts == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    labels->states = PUSH_LT(lt, malloc(sizeof(float) * labels->count), free);
    if (labels->states == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    labels->visible = PUSH_LT(lt, malloc(sizeof(int) * labels->count), free);
    if (labels->visible == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    char color[7];
    for (size_t i = 0; i < labels->count; ++i) {
        labels->states[i] = 1.0f;
        labels->visible[i] = 0;

        labels->texts[i] = PUSH_LT(lt, malloc(sizeof(char) * (LABEL_TEXT_MAX_LENGTH + 1)), free);
        if (labels->texts[i] == NULL) {
            throw_error(ERROR_TYPE_LIBC);
            RETURN_LT(lt, NULL);
        }

        if (fscanf(stream, "%f%f%6s\n",
                   &labels->positions[i].x,
                   &labels->positions[i].y,
                   color) == EOF) {
            throw_error(ERROR_TYPE_LIBC);
            RETURN_LT(lt, NULL);
        }

        labels->colors[i] = color_from_hexstr(color);

        if (fgets(labels->texts[i], LABEL_TEXT_MAX_LENGTH, stream) == NULL) {
            throw_error(ERROR_TYPE_LIBC);
            RETURN_LT(lt, NULL);
        }

        trim_endline(labels->texts[i]);
    }

    return labels;
}

void destroy_labels(Labels *label)
{
    assert(label);
    RETURN_LT0(label->lt);
}

int labels_render(const Labels *label,
                 Camera *camera)
{
    assert(label);
    assert(camera);

    for (size_t i = 0; i < label->count; ++i) {
        if (label->visible[i]) {
            /* Easing */
            const float state = label->states[i] * (2 - label->states[i]);

            if (camera_render_text(camera,
                                   label->texts[i],
                                   vec(2.0f, 2.0f),
                                   color(label->colors[i].r,
                                         label->colors[i].g,
                                         label->colors[i].b,
                                         state),
                                   vec_sum(label->positions[i],
                                           vec(0.0f, -8.0f * state))) < 0) {
                return -1;
            }
        }
    }

    return 0;
}

void labels_update(Labels *label,
                   float delta_time)
{
    assert(label);
    (void) delta_time;

    for (size_t i = 0; i < label->count; ++i) {
        label->states[i] = fminf(label->states[i] + delta_time, 1.0f);
    }
}

void labels_enter_camera_event(Labels *labels,
                               const Camera *camera)
{
    assert(labels);
    assert(camera);

    for (size_t i = 0; i < labels->count; ++i) {
        const int became_visible = camera_is_text_visible(
            camera,
            vec(2.0f, 2.0f),
            labels->positions[i],
            labels->texts[i]);

        if (!labels->visible[i] && became_visible) {
            labels->states[i] = 0.0f;
        }

        labels->visible[i] = became_visible;
    }
}
