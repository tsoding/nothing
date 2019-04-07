#ifndef LEVEL_EDITOR_H_
#define LEVEL_EDITOR_H_

typedef struct LevelEditor LevelEditor;
typedef struct Boxes Boxes;

LevelEditor *create_level_editor(Boxes *boxes);
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

#endif  // LEVEL_EDITOR_H_
