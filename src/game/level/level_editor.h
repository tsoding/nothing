#ifndef LEVEL_EDITOR_H_
#define LEVEL_EDITOR_H_

typedef struct LevelEditor LevelEditor;

LevelEditor *create_level_editor(Layer *boxes_layer);
void destroy_level_editor(LevelEditor *level_editor);

int level_editor_render(const LevelEditor *level_editor,
                        Camera *camera);
int level_editor_update(LevelEditor *level_editor,
                        float delta_time);
int level_editor_event(LevelEditor *level_editor,
                       const SDL_Event *event,
                       const Camera *camera);
int level_editor_focus_camera(LevelEditor *level_editor,
                              Camera *camera);

const Layer *level_editor_boxes(const LevelEditor *level_editor);

#endif  // LEVEL_EDITOR_H_
