#include <assert.h>

#include "game/camera.h"
#include "game/level/labels.h"
#include "system/error.h"
#include "system/lt.h"

#define LABEL_TEXT_MAX_LENGTH 64

struct labels_t
{
    lt_t *lt;
    size_t count;
    vec_t *positions;
    color_t *colors;
    char **texts;
    float *states;
};

static char *trim_endline(char *s)
{
    const size_t n = strlen(s);

    if (n == 0) {
        return s;
    }

    if (s[n - 1] == '\n') {
        s[n - 1] = '\0';
    }

    return s;
}

labels_t *create_labels_from_stream(FILE *stream)
{
    assert(stream);

    lt_t *const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    labels_t * const labels = PUSH_LT(lt, malloc(sizeof(labels_t)), free);
    if (labels == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    labels->lt = lt;

    if (fscanf(stream, "%lu", &labels->count) == EOF) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    labels->positions = PUSH_LT(lt, malloc(sizeof(vec_t) * labels->count), free);
    if (labels->positions == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    labels->colors = PUSH_LT(lt, malloc(sizeof(color_t) * labels->count), free);
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

    char color[7];
    for (size_t i = 0; i < labels->count; ++i) {
        labels->states[i] = 1.0f;

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

void destroy_labels(labels_t *label)
{
    assert(label);
    RETURN_LT0(label->lt);
}

int labels_render(const labels_t *label,
                 camera_t *camera)
{
    assert(label);
    assert(camera);

    /* TODO: position and transparency of label is not dependent on its state */
    for (size_t i = 0; i < label->count; ++i) {
        if (camera_render_text(camera,
                               label->texts[i],
                               vec(2.0f, 2.0f),
                               color(label->colors[i].r,
                                     label->colors[i].g,
                                     label->colors[i].b,
                                     label->states[i]),
                               vec_sum(label->positions[i],
                                       vec(0.0f, -8.0f * label->states[i]))) < 0) {
            return -1;
        }
    }

    return 0;
}

void labels_update(labels_t *label,
                   float delta_time)
{
    assert(label);
    (void) delta_time;

    for (size_t i = 0; i < label->count; ++i) {
        label->states[i] = fmodf(label->states[i] + delta_time, 1.0f);
    }
}

void labels_enter_camera_event(labels_t *labels,
                               const camera_t *camera)
{
    assert(labels);
    assert(camera);

    /* TODO: labels_enter_camera_event is not implemented */
}
