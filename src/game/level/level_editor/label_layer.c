#include <SDL.h>

#include "system/line_stream.h"
#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "system/lt.h"
#include "system/str.h"
#include "system/log.h"
#include "math/point.h"
#include "label_layer.h"
#include "dynarray.h"
#include "color.h"
#include "game/camera.h"

#define LABEL_LAYER_ID_MAX_SIZE 36

struct LabelLayer {
    Lt *lt;
    Dynarray *ids;
    Dynarray *positions;
    Dynarray *colors;
    Dynarray *texts;
};

LayerPtr label_layer_as_layer(LabelLayer *label_layer)
{
    LayerPtr layer = {
        .ptr = label_layer,
        .type = LAYER_LABEL
    };
    return layer;
}

LabelLayer *create_label_layer(void)
{
    Lt *lt = create_lt();

    LabelLayer *label_layer = PUSH_LT(
        lt, nth_calloc(1, sizeof(LabelLayer)), free);
    if (label_layer == NULL) {
        RETURN_LT(lt, NULL);
    }
    label_layer->lt = lt;

    label_layer->ids = PUSH_LT(
        lt,
        create_dynarray(sizeof(char) * LABEL_LAYER_ID_MAX_SIZE),
        destroy_dynarray);
    if (label_layer->ids == NULL) {
        RETURN_LT(lt, NULL);
    }

    label_layer->positions = PUSH_LT(lt, create_dynarray(sizeof(Point)), destroy_dynarray);
    if (label_layer->positions == NULL) {
        RETURN_LT(lt, NULL);
    }

    label_layer->colors = PUSH_LT(lt, create_dynarray(sizeof(Color)), destroy_dynarray);
    if (label_layer->colors == NULL) {
        RETURN_LT(lt, NULL);
    }

    label_layer->texts = PUSH_LT(lt, create_dynarray(sizeof(char*)), destroy_dynarray);
    if (label_layer->texts == NULL) {
        RETURN_LT(lt, NULL);
    }

    return label_layer;
}

LabelLayer *create_label_layer_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);
    LabelLayer *label_layer = create_label_layer();

    if (label_layer == NULL) {
        RETURN_LT(label_layer->lt, NULL);
    }

    const char *line = line_stream_next(line_stream);
    if (line == NULL) {
        log_fail("Could not read amount of labels\n");
        RETURN_LT(label_layer->lt, NULL);
    }

    size_t n = 0;
    if (sscanf(line, "%lu", &n) == EOF) {
        log_fail("Could not parse amount of labels\n");
        RETURN_LT(label_layer->lt, NULL);
    }

    for (size_t i = 0; i < n; ++i) {
        char hex[7];
        char id[LABEL_LAYER_ID_MAX_SIZE];
        Point position;

        line = line_stream_next(line_stream);
        if (line == NULL) {
            log_fail("Could not read label meta info\n");
            RETURN_LT(label_layer->lt, NULL);
        }

        if (sscanf(
                line,
                "%"STRINGIFY(LABEL_LAYER_ID_MAX_SIZE)"s%f%f%6s\n",
                id, &position.x, &position.y, hex) == EOF) {
            log_fail("Could not parse label meta info\n");
            RETURN_LT(label_layer->lt, NULL);
        }

        Color color = hexstr(hex);

        dynarray_push(label_layer->ids, id);
        dynarray_push(label_layer->positions, &position);
        dynarray_push(label_layer->colors, &color);

        line = line_stream_next(line_stream);
        if (line == NULL) {
            log_fail("Could not read label text\n");
        }

        char *label_text = PUSH_LT(label_layer->lt, string_duplicate(line, NULL), free);
        trim_endline(label_text);
        dynarray_push(label_layer->texts, &label_text);
    }

    return label_layer;
}

void destroy_label_layer(LabelLayer *label_layer)
{
    trace_assert(label_layer);
    destroy_lt(label_layer->lt);
}

int label_layer_render(const LabelLayer *label_layer,
                       Camera *camera,
                       float fa)
{
    trace_assert(label_layer);
    trace_assert(camera);

    size_t n = dynarray_count(label_layer->ids);
    Point *positions = dynarray_data(label_layer->positions);
    Color *colors = dynarray_data(label_layer->colors);
    char **texts = dynarray_data(label_layer->texts);

    /* TODO: LabelLayer doesn't show the final position of Label after the animation */
    for (size_t i = 0; i < n; ++i) {
        if (camera_render_text(
                camera,
                texts[i],
                vec(2.0f, 2.0f),
                color_scale(
                    colors[i],
                    rgba(1.0f, 1.0f, 1.0f, fa)),
                positions[i]) < 0) {
            return -1;
        }
    }

    return 0;
}

int label_layer_event(LabelLayer *label_layer,
                      const SDL_Event *event,
                      const Camera *camera)
{
    trace_assert(label_layer);
    trace_assert(event);
    trace_assert(camera);
    /* TODO: LabelLayer doesn't allow to modify and add labels */
    return 0;
}

size_t label_layer_count(const LabelLayer *label_layer)
{
    return dynarray_count(label_layer->ids);
}

char *label_layer_ids(const LabelLayer *label_layer)
{
    return dynarray_data(label_layer->ids);
}

Point *label_layer_positions(const LabelLayer *label_layer)
{
    return dynarray_data(label_layer->positions);
}

Color *label_layer_colors(const LabelLayer *label_layer)
{
    return dynarray_data(label_layer->colors);
}

char **labels_layer_texts(const LabelLayer *label_layer)
{
    return dynarray_data(label_layer->texts);
}
