#include <assert.h>

#include "game/camera.h"
#include "game/level/labels.h"
#include "str.h"
#include "system/line_stream.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/log.h"

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

Labels *create_labels_from_line_stream(LineStream *line_stream)
{
    assert(line_stream);

    Lt *const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Labels * const labels = PUSH_LT(lt, nth_alloc(sizeof(Labels)), free);
    if (labels == NULL) {
        RETURN_LT(lt, NULL);
    }
    labels->lt = lt;

    if (sscanf(
            line_stream_next(line_stream),
            "%lu",
            &labels->count) == EOF) {
        log_fail("Could not read amount of labels\n");
        RETURN_LT(lt, NULL);
    }

    labels->positions = PUSH_LT(lt, nth_alloc(sizeof(Vec) * labels->count), free);
    if (labels->positions == NULL) {
        RETURN_LT(lt, NULL);
    }

    labels->colors = PUSH_LT(lt, nth_alloc(sizeof(Color) * labels->count), free);
    if (labels->colors == NULL) {
        RETURN_LT(lt, NULL);
    }

    labels->texts = PUSH_LT(lt, nth_alloc(sizeof(char*) * labels->count), free);
    if (labels->texts == NULL) {
        RETURN_LT(lt, NULL);
    }

    labels->states = PUSH_LT(lt, nth_alloc(sizeof(float) * labels->count), free);
    if (labels->states == NULL) {
        RETURN_LT(lt, NULL);
    }

    labels->visible = PUSH_LT(lt, nth_alloc(sizeof(int) * labels->count), free);
    if (labels->visible == NULL) {
        RETURN_LT(lt, NULL);
    }

    char color[7];
    for (size_t i = 0; i < labels->count; ++i) {
        labels->states[i] = 1.0f;
        labels->visible[i] = 0;
        labels->texts[i] = NULL;

        if (sscanf(
                line_stream_next(line_stream),
                "%f%f%6s\n",
                &labels->positions[i].x,
                &labels->positions[i].y,
                color) == EOF) {
            log_fail("Could not read position and color of %dth label\n", i);
            RETURN_LT(lt, NULL);
        }

        labels->colors[i] = hexstr(color);

        const char *label_text = line_stream_next(line_stream);
        if (label_text == NULL) {
            log_fail("Could not read text of %dth label\n", i);
            RETURN_LT(lt, NULL);
        }

        labels->texts[i] = PUSH_LT(
            lt,
            string_duplicate(label_text, NULL),
            free);
        if (labels->texts[i] == NULL) {
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
                                   rgba(label->colors[i].r,
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
