#ifndef LEVEL_PICKER_H_
#define LEVEL_PICKER_H_

#include <SDL2/SDL.h>

typedef struct LevelPicker LevelPicker;
typedef struct Camera Camera;

LevelPicker *create_level_picker(const char *dirpath);
void destroy_level_picker(LevelPicker *level_picker);

int level_picker_render(const LevelPicker *level_picker,
                        Camera *camera,
                        SDL_Renderer *renderer);
int level_picker_update(LevelPicker *level,
                        float delta_time);
int level_picker_event(LevelPicker *level_picker, const SDL_Event *event);
int level_picker_input(LevelPicker *level_picker,
                       const Uint8 *const keyboard_state,
                       SDL_Joystick *the_stick_of_joy);
int level_picker_enter_camera_event(LevelPicker *level_picker,
                                    Camera *camera);

const char *level_picker_selected_level(const LevelPicker *level_picker);
void level_picker_clean_selection(LevelPicker *level_picker);

#endif  // LEVEL_PICKER_H_
