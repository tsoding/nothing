#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "./player.h"
#include "./platforms.h"

#define PLAYER_WIDTH 50.0f
#define PLAYER_HEIGHT 50.0f
#define PLAYER_SPEED 500.0f
#define PLAYER_GRAVITY 1500.0f

struct player_t {
    vec_t position;
    vec_t velocity;
    vec_t movement;
};

/* static const vec_t impact_vecs[RECT_SIDE_N] = { */
/*     /\* left *\/ */
/*     { .x = 1.0f, .y = 0.0f }, */
/*     /\* right *\/ */
/*     { .x = -1.0f, .y = 0.0f }, */
/*     /\* top *\/ */
/*     { .x = 0.0f, .y = 1.0f }, */
/*     /\* bottom *\/ */
/*     { .x = 0.0f, .y = -1.0f } */
/* }; */

player_t *create_player(float x, float y)
{
    player_t *player = malloc(sizeof(player_t));

    if (player == NULL) {
        return NULL;
    }

    player->position.x = x;
    player->position.y = y;
    player->velocity.x = 0.0f;
    player->velocity.y = 0.0f;
    player->movement.x = 0.0f;
    player->movement.y = 0.0f;

    return player;
}

void destroy_player(player_t * player)
{
    free(player);
}

int render_player(const player_t * player,
                  SDL_Renderer *renderer,
                  const camera_t *camera)
{
    if (SDL_SetRenderDrawColor(renderer, 96, 255, 96, 255) < 0) {
        return -1;
    }
    rect_t player_object = {
        .x = player->position.x,
        .y = player->position.y,
        .w = PLAYER_WIDTH,
        .h = PLAYER_HEIGHT
    };


    return camera_fill_rect(camera, renderer, &player_object);
}

void update_player(player_t * player,
                   const platforms_t *platforms,
                   Uint32 delta_time)
{
    float d = (float) delta_time / 1000.0f;

    float x = player->position.x;
    float y = player->position.y;

    rect_t player_object = {
        .x = x + (player->velocity.x + player->movement.x) * d,
        .y = y + (player->velocity.y + player->movement.y) * d,
        .w = PLAYER_WIDTH,
        .h = PLAYER_HEIGHT
    };

    int sides[4] = {0, 0, 0, 0};

    platforms_rect_object_collide(platforms, &player_object, sides);

    if (sides[RECT_SIDE_LEFT] || sides[RECT_SIDE_RIGHT]) {
        player->velocity.x = 0.0f;
        player->movement.x = 0.0f;
    }

    if (sides[RECT_SIDE_TOP] || sides[RECT_SIDE_BOTTOM]) {
        player->velocity.y = 0.0f;
        player->movement.y = 0.0f;
    }

    player->position.x += (player->velocity.x + player->movement.x) * d;
    player->position.y += (player->velocity.y + player->movement.y) * d;
    player->velocity.y += PLAYER_GRAVITY * d;
}

void player_move_left(player_t *player)
{
    player->movement.x = -PLAYER_SPEED;
    player->movement.y = 0.0f;
}

void player_move_right(player_t *player)
{
    player->movement.x = PLAYER_SPEED;
    player->movement.y = 0.0f;
}

void player_stop(player_t *player)
{
    player->movement.x = 0.0f;
    player->movement.y = 0.0f;
}

void player_jump(player_t *player)
{
    player->velocity.y = -1000.0f;
}

void player_focus_camera(player_t *player,
                         camera_t *camera)
{
    camera_translate(camera,
                     player->position.x - 800.0f * 0.5f + PLAYER_WIDTH * 0.5f,
                     player->position.y - 600.0f * 0.5f + PLAYER_HEIGHT * 0.5f);
}
