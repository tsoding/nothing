#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "./lt.h"
#include "./player.h"
#include "./platforms.h"
#include "./point.h"
#include "./error.h"
#include "./rigid_rect.h"
#include "./dying_rect.h"

#define PLAYER_WIDTH 25.0f
#define PLAYER_HEIGHT 25.0f
#define PLAYER_SPEED 500.0f
#define PLAYER_JUMP 550.0f
#define PLAYER_DEATH_DURATION 500

typedef enum player_state_t {
    PLAYER_STATE_ALIVE = 0,
    PLAYER_STATE_DYING
} player_state_t;

struct player_t {
    lt_t *lt;
    player_state_t state;

    rigid_rect_t *alive_body;
    dying_rect_t *dying_body;

    int jump_count;
    color_t color;

    /* TODO(#110): introduce checkpoints */
    vec_t checkpoint;
};

player_t *create_player(float x, float y, color_t color)
{
    lt_t *lt = create_lt();

    if (lt == NULL) {
        return NULL;
    }

    player_t *player = PUSH_LT(lt, malloc(sizeof(player_t)), free);
    if (player == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    player->state = PLAYER_STATE_ALIVE;

    player->alive_body = PUSH_LT(
        lt,
        create_rigid_rect(
            rect(x, y, PLAYER_WIDTH, PLAYER_HEIGHT),
            color),
        destroy_rigid_rect);
    if (player->alive_body == NULL) {
        RETURN_LT(lt, NULL);
    }

    player->dying_body = PUSH_LT(
        lt,
        create_dying_rect(
            rect(x, y, PLAYER_WIDTH, PLAYER_HEIGHT),
            color,
            PLAYER_DEATH_DURATION),
        destroy_dying_rect);
    if (player->dying_body == NULL) {
        RETURN_LT(lt, NULL);
    }

    player->lt = lt;
    player->jump_count = 0;
    player->color = color;
    player->checkpoint = vec(x, y);

    return player;
}

player_t *create_player_from_stream(FILE *stream)
{
    float x = 0.0f, y = 0.0f;

    char color[7];
    if (fscanf(stream, "%f%f%6s", &x, &y, color) == EOF) {
        throw_error(ERROR_TYPE_LIBC);
        return NULL;
    }

    return create_player(x, y, color_from_hexstr(color));
}

void destroy_player(player_t * player)
{
    RETURN_LT0(player->lt);
}

int player_render(const player_t * player,
                  SDL_Renderer *renderer,
                  const camera_t *camera)
{
    assert(player);
    assert(renderer);
    assert(camera);

    switch (player->state) {
    case PLAYER_STATE_ALIVE:
        return rigid_rect_render(player->alive_body, renderer, camera);

    case PLAYER_STATE_DYING:
        return dying_rect_render(player->dying_body, renderer, camera);

    default: {}
    }

    return 0;
}

void player_update(player_t *player,
                   const platforms_t *platforms,
                   Uint32 delta_time)
{
    assert(player);
    assert(platforms);

    switch (player->state) {
    case PLAYER_STATE_ALIVE: {
        rigid_rect_update(player->alive_body, platforms, delta_time);

        if (rigid_rect_touches_ground(player->alive_body)) {
            player->jump_count = 0;
        }

        const rect_t hitbox = rigid_rect_hitbox(player->alive_body);

        if (hitbox.y > 1000.0f) {
            player->dying_body = RESET_LT(
                player->lt,
                player->dying_body,
                create_dying_rect(
                    hitbox,
                    player->color,
                    PLAYER_DEATH_DURATION));
            player->state = PLAYER_STATE_DYING;
        }
    } break;

    case PLAYER_STATE_DYING: {
        dying_rect_update(player->dying_body, delta_time);

        if (dying_rect_is_dead(player->dying_body)) {
            player->alive_body = RESET_LT(
                player->lt,
                player->alive_body,
                create_rigid_rect(
                    rect_from_vecs(
                        player->checkpoint,
                        vec(PLAYER_WIDTH, PLAYER_HEIGHT)),
                    player->color));
            player->state = PLAYER_STATE_ALIVE;
        }
    } break;

    default: {}
    }
}

void player_move_left(player_t *player)
{
    assert(player);
    rigid_rect_move(player->alive_body, vec(-PLAYER_SPEED, 0.0f));
}

void player_move_right(player_t *player)
{
    assert(player);

    rigid_rect_move(player->alive_body, vec(PLAYER_SPEED, 0.0f));
}

void player_stop(player_t *player)
{
    assert(player);

    rigid_rect_move(player->alive_body, vec(0.0f, 0.0f));
}

void player_jump(player_t *player)
{
    assert(player);
    if (player->jump_count < 2) {
        rigid_rect_jump(player->alive_body, PLAYER_JUMP);
        player->jump_count++;
    }
}

void player_focus_camera(player_t *player,
                         camera_t *camera)
{
    assert(player);
    assert(camera);

    const rect_t player_hitbox = rigid_rect_hitbox(player->alive_body);

    camera_center_at(
        camera,
        vec_sum(
            vec(player_hitbox.x, player_hitbox.y),
            vec(0.0f, -player_hitbox.h * 0.5f)));
}

void player_hide_goals(const player_t *player,
                       goals_t *goals)
{
    assert(player);
    assert(goals);
    goals_hide(goals, rigid_rect_hitbox(player->alive_body));
}
