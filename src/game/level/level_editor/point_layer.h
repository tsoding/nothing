#ifndef POINT_LAYER_H_
#define POINT_LAYER_H_

#include "math/vec.h"
#include "color.h"
#include "layer.h"
#include "dynarray.h"
#include "game/level/level_editor/color_picker.h"
#include "ui/edit_field.h"

#define ID_MAX_SIZE 36

typedef enum {
    POINT_LAYER_IDLE = 0,
    POINT_LAYER_EDIT_ID,
    POINT_LAYER_MOVE,
    POINT_LAYER_RECOLOR
} PointLayerState;

typedef struct {
    PointLayerState state;
    Dynarray/*<Vec2f>*/ positions;
    Dynarray/*<Color>*/ colors;
    Dynarray/*<char[ID_MAX_SIZE]>*/ ids;
    int selection;
    ColorPicker color_picker;

    Vec2f inter_position;
    Color inter_color;
    Edit_field edit_field;

    int id_name_counter;
    const char *id_name_prefix;
} PointLayer;


LayerPtr point_layer_as_layer(PointLayer *point_layer);
// NOTE: create_point_layer and create_point_layer_from_line_stream do
// not own id_name_prefix
PointLayer *create_point_layer(Memory *memory, const char *id_name_prefix);
void point_layer_load(PointLayer *point_layer,
                        Memory *memory,
                        String *input);

static inline
void destroy_point_layer(PointLayer point_layer)
{
    free(point_layer.positions.data);
    free(point_layer.colors.data);
    free(point_layer.ids.data);
}


int point_layer_render(const PointLayer *point_layer,
                       const Camera *camera,
                       int active);
int point_layer_event(PointLayer *point_layer,
                      const SDL_Event *event,
                      const Camera *camera,
                      UndoHistory *undo_history);

int point_layer_dump_stream(const PointLayer *point_layer,
                            FILE *filedump);

size_t point_layer_count(const PointLayer *point_layer);
const Vec2f *point_layer_positions(const PointLayer *point_layer);
const Color *point_layer_colors(const PointLayer *point_layer);
const char *point_layer_ids(const PointLayer *point_layer);

#endif  // POINT_LAYER_H_
