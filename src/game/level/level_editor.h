#ifndef LEVEL_EDITOR_H_
#define LEVEL_EDITOR_H_

typedef struct LevelEditor LevelEditor;
typedef struct RectLayer RectLayer;
typedef struct PointLayer PointLayer;

// TODO(#820): LevelEditor cannot generate a level file

LevelEditor *create_level_editor(RectLayer *boxes_layer,
                                 RectLayer *platforms_layer,
                                 RectLayer *back_platforms_layer,
                                 PointLayer *goals_layer,
                                 PlayerLayer *player_layer_render);
void destroy_level_editor(LevelEditor *level_editor);

int level_editor_render(const LevelEditor *level_editor,
                        Camera *camera);
int level_editor_event(LevelEditor *level_editor,
                       const SDL_Event *event,
                       const Camera *camera);
int level_editor_focus_camera(LevelEditor *level_editor,
                              Camera *camera);

const RectLayer *level_editor_boxes(const LevelEditor *level_editor);
const RectLayer *level_editor_platforms(const LevelEditor *level_editor);
const RectLayer *level_editor_back_platforms(const LevelEditor *level_editor);
const PointLayer *level_editor_goals_layer(const LevelEditor *level_editor);
const PlayerLayer *level_editor_player_layer(const LevelEditor *level_editor);

#endif  // LEVEL_EDITOR_H_
