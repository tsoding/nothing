#ifndef LEVEL_EDITOR_H_
#define LEVEL_EDITOR_H_

#include "game/level/level_editor/layer.h"
#include "game/level/level_editor/layer_picker.h"

typedef struct LevelMetadata LevelMetadata;
typedef struct LevelEditor LevelEditor;
typedef struct RectLayer RectLayer;
typedef struct PointLayer PointLayer;
typedef struct LabelLayer LabelLayer;

struct LevelEditor
{
    Lt *lt;
    Vec camera_position;
    float camera_scale;
    LayerPicker layer_picker;

    LevelMetadata *metadata;
    RectLayer *boxes_layer;
    RectLayer *platforms_layer;
    RectLayer *back_platforms_layer;
    PointLayer *goals_layer;
    PlayerLayer *player_layer;
    RectLayer *lava_layer;
    RectLayer *regions_layer;
    ColorPicker background_layer;
    LabelLayer *label_layer;
    char *supa_script_source;
    LayerPtr layers[LAYER_PICKER_N];

    bool drag;
};


LevelEditor *create_level_editor_from_file(const char *file_name);
void destroy_level_editor(LevelEditor *level_editor);

int level_editor_render(const LevelEditor *level_editor,
                        Camera *camera);
int level_editor_event(LevelEditor *level_editor,
                       const SDL_Event *event,
                       const Camera *camera);
int level_editor_focus_camera(LevelEditor *level_editor,
                              Camera *camera);
int level_editor_dump(const LevelEditor *level_editor,
                      const char *filename);

#endif  // LEVEL_EDITOR_H_
