#ifndef LEVEL_EDITOR_H_
#define LEVEL_EDITOR_H_

#include "game/level/level_editor/layer.h"
#include "game/level/level_editor/layer_picker.h"
#include "game/level/level_editor/undo_history.h"
#include "game/level/level_editor/action_picker.h"
#include "game/level_metadata.h"
#include "ui/wiggly_text.h"
#include "ui/cursor.h"

typedef struct LevelEditor LevelEditor;
typedef struct RectLayer RectLayer;
typedef struct PointLayer PointLayer;
typedef struct LabelLayer LabelLayer;
typedef struct Edit_field Edit_field;
typedef struct Sound_samples Sound_samples;

typedef enum {
    LEVEL_EDITOR_IDLE = 0,
    LEVEL_EDITOR_SAVEAS
} LevelEditorState;

struct LevelEditor
{
    Lt *lt;
    LevelEditorState state;
    Vec2f camera_position;
    float camera_scale;
    Edit_field *edit_field_filename;
    LayerPicker layer_picker;
    FadingWigglyText notice;

    LevelMetadata metadata;
    RectLayer *boxes_layer;
    RectLayer *platforms_layer;
    RectLayer *back_platforms_layer;
    PointLayer *goals_layer;
    PlayerLayer player_layer;
    RectLayer *lava_layer;
    RectLayer *regions_layer;
    BackgroundLayer background_layer;
    LabelLayer *label_layer;

    LayerPtr layers[LAYER_PICKER_N];

    UndoHistory undo_history;

    bool drag;
    int bell;
    int click;
    int save;

    const char *file_name;
};

LevelEditor *create_level_editor(Cursor *cursor);
LevelEditor *create_level_editor_from_file(const char *file_name, Cursor *cursor);
void destroy_level_editor(LevelEditor *level_editor);

int level_editor_render(const LevelEditor *level_editor,
                        const Camera *camera);
int level_editor_event(LevelEditor *level_editor,
                       const SDL_Event *event,
                       Camera *camera);
int level_editor_focus_camera(LevelEditor *level_editor,
                              Camera *camera);
int level_editor_update(LevelEditor *level_editor, float delta_time);
void level_editor_sound(LevelEditor *level_editor, Sound_samples *sound_samples);

#endif  // LEVEL_EDITOR_H_
