#ifndef PLAYER_H_
#define PLAYER_H_

struct player;
struct platforms_t;
struct SDL_Renderer;

struct player *create_player(float x, float y);
void destroy_player(struct player * player);

int render_player(const struct player * player,
                  SDL_Renderer *renderer);
void update_player(struct player * player,
                   const struct platforms_t *platforms,
                   int delta_time);

void player_move_left(struct player *player);
void player_move_right(struct player *player);
void player_stop(struct player *player);
void player_jump(struct player *player);

#endif  // PLAYER_H_
