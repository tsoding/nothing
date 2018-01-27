#ifndef LEVEL_H_
#define LEVEL_H_

typedef struct level_t level_t;

typedef struct player_t player_t;
typedef struct platforms_t platforms_t;
typedef struct SDL_Renderer SDL_Renderer;
typedef union SDL_Event SDL_Event;
typedef struct camera_t camera_t;

level_t *create_level_from_file(const char *file_name);
void destroy_level(level_t *level);

int level_render(const level_t *level, SDL_Renderer *renderer);
int level_update(level_t *level, Uint32 delta_time);

int level_event(level_t *level, const SDL_Event *event);
int level_input(level_t *level,
                const Uint8 *const keyboard_state,
                SDL_Joystick *the_stick_of_joy);
int level_reload_preserve_player(level_t *level,
                                 const char *file_name);

void level_toggle_debug_mode(level_t *level);
void level_toggle_pause_mode(level_t *level);

#endif  // LEVEL_H_
