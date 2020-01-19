#include <stdio.h>
#include <stdbool.h>

#include "config.h"
#include "game/camera.h"
#include "game/level/labels.h"
#include "game/level/level_editor/label_layer.h"
#include "system/log.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/stacktrace.h"
#include "system/str.h"

enum LabelState
{
    LABEL_STATE_VIRGIN = 0,
    LABEL_STATE_APPEARED,
    LABEL_STATE_HIDDEN
};

struct Labels
{
    Lt *lt;
    size_t count;
    char *ids;
    Vec2f *positions;
    Color *colors;
    char **texts;

    /* Animation state */
    float *alphas;
    float *delta_alphas;
    enum LabelState *states;
};

Labels *create_labels_from_label_layer(const LabelLayer *label_layer)
{
    trace_assert(label_layer);

    Lt *lt = create_lt();

    Labels *labels = PUSH_LT(lt, nth_calloc(1, sizeof(Labels)), free);
    if (labels == NULL) {
        RETURN_LT(lt, NULL);
    }
    labels->lt = lt;

    labels->count = label_layer_count(label_layer);

    labels->ids = PUSH_LT(lt, nth_calloc(labels->count, sizeof(char) * ENTITY_MAX_ID_SIZE), free);
    if (labels->ids == NULL) {
        RETURN_LT(lt, NULL);
    }
    memcpy(labels->ids,
           label_layer_ids(label_layer),
           labels->count * sizeof(char) * ENTITY_MAX_ID_SIZE);

    labels->positions = PUSH_LT(lt, nth_calloc(1, sizeof(Vec2f) * labels->count), free);
    if (labels->positions == NULL) {
        RETURN_LT(lt, NULL);
    }
    memcpy(labels->positions,
           label_layer_positions(label_layer),
           labels->count * sizeof(Vec2f));

    labels->colors = PUSH_LT(lt, nth_calloc(1, sizeof(Color) * labels->count), free);
    if (labels->colors == NULL) {
        RETURN_LT(lt, NULL);
    }
    memcpy(labels->colors,
           label_layer_colors(label_layer),
           labels->count * sizeof(Color));

    labels->texts = PUSH_LT(lt, nth_calloc(1, sizeof(char*) * labels->count), free);
    if (labels->texts == NULL) {
        RETURN_LT(lt, NULL);
    }

    char *texts = labels_layer_texts(label_layer);
    for (size_t i = 0; i < labels->count; ++i) {
        labels->texts[i] = PUSH_LT(
            labels->lt,
            string_duplicate(texts + i * LABEL_LAYER_TEXT_MAX_SIZE, NULL),
            free);
    }

    labels->alphas = PUSH_LT(lt, nth_calloc(1, sizeof(float) * labels->count), free);
    if (labels->alphas == NULL) {
        RETURN_LT(lt, NULL);
    }

    labels->delta_alphas = PUSH_LT(lt, nth_calloc(1, sizeof(float) * labels->count), free);
    if (labels->delta_alphas == NULL) {
        RETURN_LT(lt, NULL);
    }

    labels->states = PUSH_LT(lt, nth_calloc(1, sizeof(enum LabelState) * labels->count), free);
    if (labels->states == NULL) {
        RETURN_LT(lt, NULL);
    }

    return labels;
}

void destroy_labels(Labels *label)
{
    trace_assert(label);
    RETURN_LT0(label->lt);
}

int labels_render(const Labels *label,
                  const Camera *camera)
{
    trace_assert(label);
    trace_assert(camera);

    for (size_t i = 0; i < label->count; ++i) {
        /* Easing */
        const float state = label->alphas[i] * (2 - label->alphas[i]);

        if (camera_render_text(camera,
                               label->texts[i],
                               LABELS_SIZE,
                               rgba(label->colors[i].r,
                                    label->colors[i].g,
                                    label->colors[i].b,
                                    state),
                               vec_sum(label->positions[i],
                                       vec(0.0f, -8.0f * state))) < 0) {
            return -1;
        }
    }

    return 0;
}

void labels_update(Labels *label,
                   float delta_time)
{
    trace_assert(label);
    (void) delta_time;

    for (size_t i = 0; i < label->count; ++i) {
        label->alphas[i] = label->alphas[i] + label->delta_alphas[i] * delta_time;

        if (label->alphas[i] < 0.0f) {
            label->alphas[i] = 0.0f;
            label->delta_alphas[i] = 0.0f;
        }

        if (label->alphas[i] > 1.0f) {
            label->alphas[i] = 1.0f;
            label->delta_alphas[i] = 0.0f;
        }
    }
}

void labels_enter_camera_event(Labels *labels,
                               const Camera *camera)
{
    trace_assert(labels);
    trace_assert(camera);

    for (size_t i = 0; i < labels->count; ++i) {
        const int became_visible = camera_is_text_visible(
            camera,
            vec(2.0f, 2.0f),
            labels->positions[i],
            labels->texts[i]);

        if (labels->states[i] == LABEL_STATE_VIRGIN && became_visible) {
            labels->states[i] = LABEL_STATE_APPEARED;
            labels->alphas[i] = 0.0f;
            labels->delta_alphas[i] = 1.0f;
        }
    }
}

void labels_hide(Labels *labels, char id[ENTITY_MAX_ID_SIZE])
{
    trace_assert(labels);
    trace_assert(id);

    for (size_t i = 0; i < labels->count; ++i) {
        if (strncmp(id, labels->ids + i * ENTITY_MAX_ID_SIZE, ENTITY_MAX_ID_SIZE) == 0) {
            if (labels->states[i] != LABEL_STATE_HIDDEN) {
                labels->states[i] = LABEL_STATE_HIDDEN;
                labels->alphas[i] = 1.0f;
                labels->delta_alphas[i] = -3.0f;
            }
            return;
        }
    }
}
