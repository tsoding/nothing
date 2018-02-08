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

#define PLAYER_WIDTH 25.0f
#define PLAYER_HEIGHT 25.0f
#define PLAYER_SPEED 500.0f
#define PLAYER_JUMP 550.0f

struct player_t {
    lt_t *lt;
    rigid_rect_t *player_body;
    /* TODO(#105): fix player jump_count */
    int jump_count;
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

    player->player_body = PUSH_LT(
        lt,
        create_rigid_rect(
            rect(x, y, PLAYER_WIDTH, PLAYER_HEIGHT),
            color),
        destroy_rigid_rect);
    if (player->player_body == NULL) {
        RETURN_LT(lt, NULL);
    }

    player->lt = lt;
    player->jump_count = 0;

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
    return rigid_rect_render(player->player_body, renderer, camera);
}

void player_update(player_t *player,
                   const platforms_t *platforms,
                   Uint32 delta_time)
{
    assert(player);
    assert(platforms);
    rigid_rect_update(player->player_body, platforms, delta_time);
}

void player_move_left(player_t *player)
{
    assert(player);
    rigid_rect_move(player->player_body, vec(-PLAYER_SPEED, 0.0f));
}

void player_move_right(player_t *player)
{
    assert(player);

    rigid_rect_move(player->player_body, vec(PLAYER_SPEED, 0.0f));
}

void player_stop(player_t *player)
{
    assert(player);

    rigid_rect_move(player->player_body, vec(0.0f, 0.0f));
}

void player_jump(player_t *player)
{
    assert(player);
    rigid_rect_jump(player->player_body, PLAYER_JUMP);
}

void player_focus_camera(player_t *player,
                         camera_t *camera)
{
    assert(player);
    assert(camera);

    const rect_t player_hitbox = rigid_rect_hitbox(player->player_body);

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
    goals_hide(goals, rigid_rect_hitbox(player->player_body));
}
