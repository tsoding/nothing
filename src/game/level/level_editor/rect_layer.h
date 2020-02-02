#ifndef RECT_LAYER_H_
#define RECT_LAYER_H_

#include "layer.h"
#include "game/level/action.h"
#include "ui/cursor.h"
#include "dynarray.h"
#include "color_picker.h"
#include "ui/edit_field.h"

typedef struct RectLayer RectLayer;

typedef enum {
    RECT_LAYER_IDLE = 0,
    RECT_LAYER_CREATE,
    RECT_LAYER_RESIZE,
    RECT_LAYER_MOVE,
    RECT_LAYER_ID_RENAME,
    RECT_LAYER_RECOLOR,
    RECT_LAYER_SUBTRACT
} RectLayerState;

struct RectLayer {
    RectLayerState state;
    int resize_mask;
    Dynarray ids;
    Dynarray rects;
    Dynarray colors;
    Dynarray actions;
    ColorPicker color_picker;
    Vec2f create_begin;
    Vec2f create_end;
    int selection;
    Vec2f move_anchor;          // The mouse offset from the left-top
                                // corner of the rect during moving it
    Edit_field id_edit_field;
    Color inter_color;
    Rect inter_rect;
    int id_name_counter;
    const char *id_name_prefix;
    Cursor *cursor;

    int snapping_enabled;
    int subtract_enabled;
};

LayerPtr rect_layer_as_layer(RectLayer *layer);
// NOTE: create_rect_layer and create_rect_layer_from_line_stream does
// not own id_name_prefix

RectLayer *create_rect_layer(Memory *memory,
                             const char *id_name_prefix,
                             Cursor *cursor);
void rect_layer_load(RectLayer *rect_layer, Memory *memory, String *input);

static inline
void destroy_rect_layer(RectLayer layer)
{
    free(layer.ids.data);
    free(layer.rects.data);
    free(layer.colors.data);
    free(layer.actions.data);
}


int rect_layer_render(const RectLayer *layer, const Camera *camera, int active);
int rect_layer_event(RectLayer *layer,
                     const SDL_Event *event,
                     const Camera *camera,
                     UndoHistory *undo_history);

int rect_layer_dump_stream(const RectLayer *layer, FILE *filedump);

size_t rect_layer_count(const RectLayer *layer);
const Rect *rect_layer_rects(const RectLayer *layer);
const Color *rect_layer_colors(const RectLayer *layer);
const char *rect_layer_ids(const RectLayer *layer);
const Action *rect_layer_actions(const RectLayer *layer);

#endif  // RECT_LAYER_H_
