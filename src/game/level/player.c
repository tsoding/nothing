#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "game/level/player/dying_rect.h"
#include "game/level/player/rigid_rect.h"
#include "goals.h"
#include "math/point.h"
#include "platforms.h"
#include "player.h"
#include "system/error.h"
#include "system/lt.h"
#include "system/line_stream.h"

#define PLAYER_WIDTH 25.0f
#define PLAYER_HEIGHT 25.0f
#define PLAYER_SPEED 500.0f
#define PLAYER_JUMP 32000.0f
#define PLAYER_DEATH_DURATION 0.75f
#define PLAYER_MAX_JUMP_COUNT 2

typedef enum Player_state {
    PLAYER_STATE_ALIVE = 0,
    PLAYER_STATE_DYING
} Player_state;

struct Player {
    Lt *lt;
    Player_state state;

    Rigid_rect *alive_body;
    Dying_rect *dying_body;

    int jump_count;
    Color color;

    Vec checkpoint;

    int play_die_cue;
};

Player *create_player(float x, float y, Color color)
{
    Lt *lt = create_lt();

    if (lt == NULL) {
        return NULL;
    }

    Player *player = PUSH_LT(lt, malloc(sizeof(Player)), free);
    if (player == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    player->state = PLAYER_STATE_ALIVE;

    player->alive_body = PUSH_LT(
        lt,
        create_rigid_rect(
            rect(x, y, PLAYER_WIDTH, PLAYER_HEIGHT),
            color,
            "player"),
        destroy_rigid_rect);
    if (player->alive_body == NULL) {
        RETURN_LT(lt, NULL);
    }

    player->dying_body = PUSH_LT(
        lt,
        create_dying_rect(
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
    player->play_die_cue = 0;

    return player;
}

Player *create_player_from_line_stream(LineStream *line_stream)
{
    float x = 0.0f, y = 0.0f;
    char color[7];

    if (sscanf(
            line_stream_next(line_stream),
            "%f%f%6s",
            &x, &y, color) == EOF) {
        throw_error(ERROR_TYPE_LIBC);
        return NULL;
    }

    return create_player(x, y, color_from_hexstr(color));
}

void destroy_player(Player * player)
{
    RETURN_LT0(player->lt);
}

Solid_ref player_as_solid(Player *player)
{
    Solid_ref ref = {
        .tag = SOLID_PLAYER,
        .ptr = (void*) player
    };

    return ref;
}

int player_render(const Player * player,
                  Camera *camera)
{
    assert(player);
    assert(camera);

    switch (player->state) {
    case PLAYER_STATE_ALIVE:
        return rigid_rect_render(player->alive_body, camera);

    case PLAYER_STATE_DYING:
        return dying_rect_render(player->dying_body, camera);

    default: {}
    }

    return 0;
}

void player_update(Player *player,
                   float delta_time)
{
    assert(player);

    switch (player->state) {
    case PLAYER_STATE_ALIVE: {
        rigid_rect_update(player->alive_body, delta_time);

        const Rect hitbox = rigid_rect_hitbox(player->alive_body);

        if (hitbox.y > 1000.0f) {
            player_die(player);
        }
    } break;

    case PLAYER_STATE_DYING: {
        dying_rect_update(player->dying_body, delta_time);

        if (dying_rect_is_dead(player->dying_body)) {
            rigid_rect_transform_velocity(
                player->alive_body,
                make_mat3x3(0.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 1.0f));
            rigid_rect_teleport_to(player->alive_body, player->checkpoint);
            player->state = PLAYER_STATE_ALIVE;
        }
    } break;

    default: {}
    }
}

void player_collide_with_solid(Player *player, Solid_ref solid)
{
    if (player->state == PLAYER_STATE_ALIVE) {
        rigid_rect_collide_with_solid(player->alive_body, solid);

        if (rigid_rect_touches_ground(player->alive_body)) {
            player->jump_count = 0;
        }
    }
}

void player_move_left(Player *player)
{
    assert(player);
    rigid_rect_move(player->alive_body, vec(-PLAYER_SPEED, 0.0f));
}

void player_move_right(Player *player)
{
    assert(player);

    rigid_rect_move(player->alive_body, vec(PLAYER_SPEED, 0.0f));
}

void player_stop(Player *player)
{
    assert(player);

    rigid_rect_move(player->alive_body, vec(0.0f, 0.0f));
}

void player_jump(Player *player)
{
    assert(player);
    if (player->jump_count < PLAYER_MAX_JUMP_COUNT) {
        rigid_rect_transform_velocity(player->alive_body,
                                      make_mat3x3(1.0f, 0.0f, 0.0f,
                                                  0.0f, 0.0f, 0.0f,
                                                  0.0f, 0.0f, 1.0f));
        rigid_rect_apply_force(player->alive_body,
                               vec(0.0f, -PLAYER_JUMP));
        player->jump_count++;
    }
}

void player_die(Player *player)
{
    assert(player);

    if (player->state == PLAYER_STATE_ALIVE) {
        const Rect hitbox =
            rigid_rect_hitbox(player->alive_body);

        player->play_die_cue = 1;
        dying_rect_start_dying(player->dying_body, vec(hitbox.x, hitbox.y));
        player->state = PLAYER_STATE_DYING;
    }
}

void player_focus_camera(Player *player,
                         Camera *camera)
{
    assert(player);
    assert(camera);

    const Rect player_hitbox = rigid_rect_hitbox(player->alive_body);

    camera_center_at(
        camera,
        vec_sum(
            vec(player_hitbox.x, player_hitbox.y),
            vec(0.0f, -player_hitbox.h * 0.5f)));
}

void player_hide_goals(const Player *player,
                       Goals *goals)
{
    assert(player);
    assert(goals);
    goals_hide(goals, rigid_rect_hitbox(player->alive_body));
}

void player_die_from_lava(Player *player,
                          const Lava *lava)
{
    if (lava_overlaps_rect(lava, rigid_rect_hitbox(player->alive_body))) {
        player_die(player);
    }
}

void player_checkpoint(Player *player, Vec checkpoint)
{
    player->checkpoint = checkpoint;
}

int player_sound(Player *player,
                 Sound_samples *sound_samples)
{
    if (player->play_die_cue) {
        player->play_die_cue = 0;

        if (sound_samples_play_sound(sound_samples, 0, 0) < 0) {
            return -1;
        }
    }

    return 0;
}

void player_touches_rect_sides(Player *player,
                               Rect object,
                               int sides[RECT_SIDE_N])
{
    if (player->state == PLAYER_STATE_ALIVE) {
        rigid_rect_touches_rect_sides(player->alive_body, object, sides);
    }
}

void player_apply_force(Player *player, Vec force)
{
    if (player->state == PLAYER_STATE_ALIVE) {
        rigid_rect_apply_force(player->alive_body, force);
    }
}

Rigid_rect *player_rigid_rect(Player *player, const char *id)
{
    assert(player);
    assert(id);

    if (player->state == PLAYER_STATE_ALIVE) {
        if (rigid_rect_has_id(player->alive_body, id)) {
            return player->alive_body;
        }
    }

    return NULL;
}

bool player_overlaps_rect(const Player *player,
                          Rect rect)
{
    assert(player);

    return player->state == PLAYER_STATE_ALIVE
        && rects_overlap(
            rect, rigid_rect_hitbox(
                player->alive_body));
}
