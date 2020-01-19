#ifndef LABEL_LAYER_H_
#define LABEL_LAYER_H_

#include "layer.h"
#include "color.h"
#include "math/vec.h"
#include "dynarray.h"
#include "game/level/level_editor/color_picker.h"
#include "ui/edit_field.h"

#define LABELS_SIZE vec(2.0f, 2.0f)
#define LABEL_LAYER_ID_MAX_SIZE 36
#define LABEL_LAYER_TEXT_MAX_SIZE 256

typedef enum {
    LABEL_LAYER_IDLE = 0,
    LABEL_LAYER_MOVE,
    LABEL_LAYER_EDIT_TEXT,
    LABEL_LAYER_EDIT_ID,
    LABEL_LAYER_RECOLOR
} LabelLayerState;

typedef struct {
    LabelLayerState state;
    Dynarray ids;
    Dynarray positions;
    Dynarray colors;
    Dynarray texts;
    int selection;
    ColorPicker color_picker;
    Vec2f move_anchor;
    Edit_field edit_field;
    Vec2f inter_position;
    Color inter_color;
    int id_name_counter;
    const char *id_name_prefix;
} LabelLayer;

LayerPtr label_layer_as_layer(LabelLayer *label_layer);

// NOTE: create_label_layer and create_label_layer_from_line_stream do
// not own id_name_prefix
LabelLayer *create_label_layer(Memory *memory, const char *id_name_prefix);
void label_layer_load(LabelLayer *label_layer,
                        Memory *memory,
                        String *input);

static inline
void destroy_label_layer(LabelLayer label_layer)
{
    free(label_layer.ids.data);
    free(label_layer.positions.data);
    free(label_layer.colors.data);
    free(label_layer.texts.data);
}

int label_layer_render(const LabelLayer *label_layer,
                       const Camera *camera,
                       int active);
int label_layer_event(LabelLayer *label_layer,
                      const SDL_Event *event,
                      const Camera *camera,
                      UndoHistory *undo_history);

size_t label_layer_count(const LabelLayer *label_layer);

int label_layer_dump_stream(const LabelLayer *label_layer, FILE *filedump);

char *label_layer_ids(const LabelLayer *label_layer);
Vec2f *label_layer_positions(const LabelLayer *label_layer);
Color *label_layer_colors(const LabelLayer *label_layer);
char *labels_layer_texts(const LabelLayer *label_layer);

#endif  // LABEL_LAYER_H_
