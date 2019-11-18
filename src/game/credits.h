#ifndef CREDITS_H_
#define CREDITS_H_

#include <SDL.h>

#include "game/camera.h"

typedef struct Credits Credits;
typedef struct Sprite_font Sprite_font;

Credits *create_credits(const Sprite_font *sprite_font);
void destroy_credits(Credits *credits);

int credits_render(const Credits *credits,
                        const Camera *camera);
int credits_update(Credits *level,
                        float delta_time);
int credits_event(Credits *credits,
                       const SDL_Event *event,
                       const Camera *camera);
int credits_input(Credits *credits,
                       const Uint8 *const keyboard_state,
                       SDL_Joystick *the_stick_of_joy);
int credits_enter_camera_event(Credits *credits,
                                    Camera *camera);

const char *credits_selected_contributor(const Credits *credits);
void credits_clean_selection(Credits *credits);


#endif  // CREDITS_H_
