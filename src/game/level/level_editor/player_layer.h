#ifndef PLAYER_LAYER_H_
#define PLAYER_LAYER_H_

typedef struct {
    Vec position;
    Color color;
} PlayerLayer;

int player_layer_render(const PlayerLayer *player_layer,
                        Camera *camera);
int player_layer_mouse_button(PlayerLayer *player_layer,
                              const SDL_MouseButtonEvent *mouse_button);
int player_layer_mouse_motion(PlayerLayer *player_layer,
                              const SDL_MouseMotionEvent *mouse_motion);

#endif  // PLAYER_LAYER_H_
