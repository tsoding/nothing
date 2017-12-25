#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "./player.h"
#include "./platforms.h"
#include "./point.h"
#include "./error.h"

#define PLAYER_WIDTH 50.0f
#define PLAYER_HEIGHT 50.0f
#define PLAYER_SPEED 500.0f
#define PLAYER_GRAVITY 1500.0f

struct player_t {
    vec_t position;
    vec_t velocity;
    vec_t movement;
    float height;
    float width;
};

static const vec_t opposing_rect_side_forces[RECT_SIDE_N] = {
    { .x = 1.0f,  .y =  0.0f  },  /* RECT_SIDE_LEFT = 0, */
    { .x = -1.0f, .y =  0.0f  },  /* RECT_SIDE_RIGHT, */
    { .x = 0.0f,  .y =  1.0f, },  /* RECT_SIDE_TOP, */
    { .x = 0.0f,  .y = -1.0f, }   /* RECT_SIDE_BOTTOM, */
};

static vec_t opposing_force_by_sides(int sides[RECT_SIDE_N])
{
    vec_t opposing_force = {
        .x = 0.0f,
        .y = 0.0f
    };

    for (rect_side_t side = 0; side < RECT_SIDE_N; ++side) {
        if (sides[side]) {
            vec_add(
                &opposing_force,
                opposing_rect_side_forces[side]);
        }
    }

    return opposing_force;
}

player_t *create_player(float x, float y)
{
    player_t *player = malloc(sizeof(player_t));

    if (player == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        return NULL;
    }

    player->position.x = x;
    player->position.y = y;
    player->velocity.x = 0.0f;
    player->velocity.y = 0.0f;
    player->movement.x = 0.0f;
    player->movement.y = 0.0f;
    player->height = PLAYER_HEIGHT;
    player->width = PLAYER_WIDTH;

    return player;
}

void destroy_player(player_t * player)
{
    free(player);
}

rect_t player_hitbox(const player_t *player)
{
    rect_t hitbox = {
        .x = player->position.x - player->width / 2,
        .y = player->position.y - player->height,
        .w = player->width,
        .h = player->height
    };

    return hitbox;
}

int render_player(const player_t * player,
                  SDL_Renderer *renderer,
                  const camera_t *camera)
{
    assert(player);
    assert(renderer);
    assert(camera);

    if (SDL_SetRenderDrawColor(renderer, 96, 255, 96, 255) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }
    rect_t player_object = player_hitbox(player);


    return camera_fill_rect(camera, renderer, &player_object);
}

void update_player(player_t *player,
                   const platforms_t *platforms,
                   Uint32 delta_time)
{
    assert(player);
    assert(platforms);

    float d = (float) delta_time / 1000.0f;

    player->velocity.y += PLAYER_GRAVITY * d;
    player->position = vec_sum(
        player->position,
        vec_scala_mult(
            vec_sum(
                player->velocity,
                player->movement),
            d));
    player->position.y = fmodf(player->position.y, 800.0f);

    int sides[RECT_SIDE_N] = { 0, 0, 0, 0 };

    platforms_rect_object_collide(platforms, player_hitbox(player), sides);
    vec_t opposing_force = opposing_force_by_sides(sides);

    for (int i = 0; i < 1000 && vec_length(opposing_force) > 1e-6; ++i) {
        player->position = vec_sum(
            player->position,
            vec_scala_mult(
                opposing_force,
                1e-2f));

        if (fabs(opposing_force.x) > 1e-6 && (opposing_force.x < 0.0f) != ((player->velocity.x + player->movement.x) < 0.0f)) {
            player->velocity.x = 0.0f;
            player->movement.x = 0.0f;
        }

        if (fabs(opposing_force.y) > 1e-6 && (opposing_force.y < 0.0f) != ((player->velocity.y + player->movement.y) < 0.0f)) {
            player->velocity.y = 0.0f;
            player->movement.y = 0.0f;
        }

        platforms_rect_object_collide(
            platforms,
            player_hitbox(player),
            sides);
        opposing_force = opposing_force_by_sides(sides);
    }
}

void player_move_left(player_t *player)
{
    assert(player);

    player->movement.x = -PLAYER_SPEED;
    player->movement.y = 0.0f;
}

void player_move_right(player_t *player)
{
    assert(player);

    player->movement.x = PLAYER_SPEED;
    player->movement.y = 0.0f;
}

void player_stop(player_t *player)
{
    assert(player);

    player->movement.x = 0.0f;
    player->movement.y = 0.0f;
}

void player_jump(player_t *player)
{
    assert(player);

    player->velocity.y = -500.0f;
}

void player_focus_camera(player_t *player,
                         camera_t *camera)
{
    assert(player);
    assert(camera);

    camera_center_at(camera, player->position);
}
