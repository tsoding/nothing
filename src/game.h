#ifndef GAME_H_
#define GAME_H_

typedef struct game_t game_t;
typedef struct SDL_Renderer SDL_Renderer;

game_t *create_game(const char *platforms_file_path);
void destroy_game(game_t *game);

int game_render(const game_t *game, SDL_Renderer *renderer);
int game_update(game_t *game, Uint32 delta_time);

int game_event(game_t *game, const SDL_Event *event);
int game_input(game_t *game,
               const Uint8 *const keyboard_state,
               SDL_Joystick *the_stick_of_joy);

int is_game_over(const game_t *game);

#endif  // GAME_H_
