#include "system/stacktrace.h"
#include <stdbool.h>

#include "game/camera.h"
#include "game/level/labels.h"
#include "system/str.h"
#include "system/line_stream.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/log.h"
#include "ebisp/interpreter.h"
#include "broadcast.h"

#define LABEL_MAX_ID_SIZE 36

enum LabelState
{
    LABEL_STATE_VIRGIN = 0,
    LABEL_STATE_APPEARED,
    LABEL_STATE_HIDDEN
};

struct Labels
{
    Lt lt;
    size_t count;
    char **ids;
    Vec *positions;
    Color *colors;
    char **texts;
    float *alphas;
    float *delta_alphas;
    enum LabelState *states;
};

Labels *create_labels_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);

    Lt const lt = {0};
    if (lt == NULL) {
        return NULL;
    }

    Labels * const labels = PUSH_LT(lt, nth_calloc(1, sizeof(Labels)), free);
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

    labels->ids = PUSH_LT(lt, nth_calloc(1, sizeof(char*) * labels->count), free);
    if (labels->ids == NULL) {
        RETURN_LT(lt, NULL);
    }

    labels->positions = PUSH_LT(lt, nth_calloc(1, sizeof(Vec) * labels->count), free);
    if (labels->positions == NULL) {
        RETURN_LT(lt, NULL);
    }

    labels->colors = PUSH_LT(lt, nth_calloc(1, sizeof(Color) * labels->count), free);
    if (labels->colors == NULL) {
        RETURN_LT(lt, NULL);
    }

    labels->texts = PUSH_LT(lt, nth_calloc(1, sizeof(char*) * labels->count), free);
    if (labels->texts == NULL) {
        RETURN_LT(lt, NULL);
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

    char color[7];
    for (size_t i = 0; i < labels->count; ++i) {
        labels->alphas[i] = 0.0f;
        labels->delta_alphas[i] = 0.0f;
        labels->states[i] = LABEL_STATE_VIRGIN;
        labels->texts[i] = NULL;

        labels->ids[i] = PUSH_LT(lt, nth_calloc(1, sizeof(char) * LABEL_MAX_ID_SIZE), free);
        if (labels->ids[i] == NULL) {
            RETURN_LT(lt, NULL);
        }

        if (sscanf(
                line_stream_next(line_stream),
                "%" STRINGIFY(LABEL_MAX_ID_SIZE) "s%f%f%6s\n",
                labels->ids[i],
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
    trace_assert(label);
    RETURN_LT0(label->lt);
}

int labels_render(const Labels *label,
                 Camera *camera)
{
    trace_assert(label);
    trace_assert(camera);

    for (size_t i = 0; i < label->count; ++i) {
        /* Easing */
        const float state = label->alphas[i] * (2 - label->alphas[i]);

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

static struct EvalResult
labels_action(Labels *labels,
              size_t index,
              Gc *gc,
              struct Scope *scope,
              struct Expr path)
{
    trace_assert(labels);
    trace_assert(gc);
    trace_assert(scope);

    const char *target = NULL;
    struct Expr rest = void_expr();
    struct EvalResult res = match_list(gc, "q*", path, &target, &rest);
    if (res.is_error) {
        return res;
    }

    if (strcmp(target, "hide") == 0) {
        if (labels->states[index] != LABEL_STATE_HIDDEN) {
            labels->states[index] = LABEL_STATE_HIDDEN;
            labels->alphas[index] = 1.0f;
            labels->delta_alphas[index] = -3.0f;
        }
        return eval_success(NIL(gc));
    }

    return unknown_target(gc, labels->ids[index], target);
}

struct EvalResult
labels_send(Labels *labels, Gc *gc, struct Scope *scope, struct Expr path)
{
    trace_assert(labels);
    trace_assert(gc);
    trace_assert(scope);

    const char *target = NULL;
    struct Expr rest = void_expr();
    struct EvalResult res = match_list(gc, "s*", path, &target, &rest);
    if (res.is_error) {
        return res;
    }

    for (size_t i = 0; i < labels->count; ++i) {
        if (strcmp(target, labels->ids[i]) == 0) {
            return labels_action(labels, i, gc, scope, rest);
        }
    }

    return unknown_target(gc, "label", target);
}
