#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "./player.h"
#include "./platforms.h"

#define PLAYER_WIDTH 50.0f
#define PLAYER_HEIGHT 50.0f
#define PLAYER_SPEED 500.0f
#define PLAYER_GRAVITY 1500.0f

struct player {
    float x, y;
    float dx, dy;
};

struct player *create_player(float x, float y)
{
    struct player *player = malloc(sizeof(struct player));

    if (player == NULL) {
        return NULL;
    }

    player->x = x;
    player->y = y;
    player->dx = 0.0f;
    player->dy = 0.0f;

    return player;
}

void destroy_player(struct player * player)
{
    free(player);
}

int render_player(const struct player * player,
                  SDL_Renderer *renderer)
{
    if (SDL_SetRenderDrawColor(renderer, 96, 255, 96, 255) < 0) {
        return -1;
    }

    SDL_Rect rect;
    rect.x = roundf(player->x);
    rect.y = roundf(player->y);
    rect.w = roundf(PLAYER_WIDTH);
    rect.h = roundf(PLAYER_HEIGHT);

    if (SDL_RenderFillRect(renderer, &rect) < 0) {
        return -1;
    }

    return 0;
}

void update_player(struct player * player,
                   const struct platforms_t *platforms,
                   int delta_time)
{
    float d = delta_time / 1000.0;

    float dx = player->dx;
    float dy = player->dy + PLAYER_GRAVITY * d;

    float x = player->x + dx * d;
    float y = fmod(player->y + dy * d, 600.0f);

    struct rect_t player_object = {
        .x = x,
        .y = y,
        .w = PLAYER_WIDTH,
        .h = PLAYER_HEIGHT
    };

    /* TODO(#6): Implement collision for the left/right sides */
    if (platforms_rect_object_collide(platforms, &player_object)) {
        dy = -player->dy * 0.75;
        x = player->x + dx * d;
        y = fmod(player->y + dy * d, 600.0f);
    }

    player->dx = dx;
    player->dy = dy;
    player->x = x;
    player->y = y;
}

void player_move_left(struct player *player)
{
    player->dx = -PLAYER_SPEED;
}

void player_move_right(struct player *player)
{
    player->dx = PLAYER_SPEED;
}

void player_stop(struct player *player)
{
    player->dx = 0.0f;
}

void player_jump(struct player *player)
{
    player->dy = -500.0f;
}
