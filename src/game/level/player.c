#include <SDL2/SDL.h>
#include "system/stacktrace.h"
#include <stdio.h>
#include <stdlib.h>

#include "game/level/player/dying_rect.h"
#include "game/level/player/rigid_rect.h"
#include "game/level/script.h"
#include "game/level/rigid_bodies.h"
#include "goals.h"
#include "math/point.h"
#include "platforms.h"
#include "player.h"
#include "system/line_stream.h"
#include "system/log.h"
#include "system/lt.h"
#include "system/nth_alloc.h"

#define PLAYER_WIDTH 25.0f
#define PLAYER_HEIGHT 25.0f
#define PLAYER_SPEED 500.0f
#define PLAYER_JUMP 32000.0f
#define PLAYER_DEATH_DURATION 0.75f
#define PLAYER_MAX_JUMP_THRESHOLD 2

typedef enum Player_state {
    PLAYER_STATE_ALIVE = 0,
    PLAYER_STATE_DYING
} Player_state;

/* TODO(#633): Player's alive_body should be a body from RigidBodies */
struct Player {
    Lt *lt;
    Player_state state;

    RigidBodies *rigid_bodies;

    RigidBodyId alive_body_id;
    Dying_rect *dying_body;
    Script *script;

    int jump_threshold;
    Color color;

    Vec checkpoint;

    int play_die_cue;
};

Player *create_player_from_line_stream(LineStream *line_stream, RigidBodies *rigid_bodies, Broadcast *broadcast)
{
    trace_assert(line_stream);

    Lt *lt = create_lt();

    if (lt == NULL) {
        return NULL;
    }

    Player *player = PUSH_LT(lt, nth_alloc(sizeof(Player)), free);
    if (player == NULL) {
        RETURN_LT(lt, NULL);
    }
    player->lt = lt;

    player->rigid_bodies = rigid_bodies;

    float x = 0.0f, y = 0.0f;
    char colorstr[7];

    if (sscanf(
            line_stream_next(line_stream),
            "%f%f%6s",
            &x, &y, colorstr) == EOF) {
        log_fail("Could not read player\n");
        RETURN_LT(lt, NULL);
    }

    player->script = PUSH_LT(
        lt,
        create_script_from_line_stream(line_stream, broadcast),
        destroy_script);
    if (player->script == NULL) {
        RETURN_LT(lt, NULL);
    }

    const Color color = hexstr(colorstr);

    player->alive_body_id = rigid_bodies_add(
        rigid_bodies,
        rect(x, y, PLAYER_WIDTH, PLAYER_HEIGHT),
        color);

    player->dying_body = PUSH_LT(
        lt,
        create_dying_rect(
            color,
            PLAYER_DEATH_DURATION),
        destroy_dying_rect);
    if (player->dying_body == NULL) {
        RETURN_LT(lt, NULL);
    }

    player->jump_threshold = 0;
    player->color = color;
    player->checkpoint = vec(x, y);
    player->play_die_cue = 0;
    player->state = PLAYER_STATE_ALIVE;

    return player;
}

void destroy_player(Player * player)
{
    RETURN_LT0(player->lt);
}

int player_render(const Player * player,
                  Camera *camera)
{
    trace_assert(player);
    trace_assert(camera);

    char debug_text[256];

    switch (player->state) {
    case PLAYER_STATE_ALIVE: {
        snprintf(debug_text, 256, "Jump: %d", player->jump_threshold);
        Rect hitbox = rigid_bodies_hitbox(player->rigid_bodies, player->alive_body_id);

        if (camera_render_debug_text(camera, debug_text, vec(hitbox.x, hitbox.y - 20.0f)) < 0) {
            return -1;
        }

        return rigid_bodies_render(player->rigid_bodies, player->alive_body_id, camera);
    }

    case PLAYER_STATE_DYING:
        return dying_rect_render(player->dying_body, camera);

    default: {}
    }

    return 0;
}

void player_update(Player *player,
                   float delta_time)
{
    trace_assert(player);

    switch (player->state) {
    case PLAYER_STATE_ALIVE: {
        rigid_bodies_update(player->rigid_bodies, player->alive_body_id, delta_time);

        const Rect hitbox = rigid_bodies_hitbox(player->rigid_bodies, player->alive_body_id);


        if (hitbox.y > 1000.0f) {
            player_die(player);
        }
    } break;

    case PLAYER_STATE_DYING: {
        dying_rect_update(player->dying_body, delta_time);

        if (dying_rect_is_dead(player->dying_body)) {
            rigid_bodies_transform_velocity(
                player->rigid_bodies,
                player->alive_body_id,
                make_mat3x3(0.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 1.0f));
            rigid_bodies_teleport_to(
                player->rigid_bodies,
                player->alive_body_id,
                player->checkpoint);
            player->state = PLAYER_STATE_ALIVE;
        }
    } break;

    default: {}
    }
}

void player_move_left(Player *player)
{
    trace_assert(player);
    rigid_bodies_move(player->rigid_bodies, player->alive_body_id, vec(-PLAYER_SPEED, 0.0f));
}

void player_move_right(Player *player)
{
    trace_assert(player);

    rigid_bodies_move(player->rigid_bodies, player->alive_body_id, vec(PLAYER_SPEED, 0.0f));
}

void player_stop(Player *player)
{
    trace_assert(player);

    rigid_bodies_move(player->rigid_bodies, player->alive_body_id, vec(0.0f, 0.0f));
}

void player_jump(Player *player)
{
    trace_assert(player);

    if (rigid_bodies_touches_ground(player->rigid_bodies, player->alive_body_id)) {
        player->jump_threshold = 0;
    }

    if (player->jump_threshold < PLAYER_MAX_JUMP_THRESHOLD) {
        rigid_bodies_transform_velocity(
            player->rigid_bodies,
            player->alive_body_id,
            make_mat3x3(1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f));
        rigid_bodies_apply_force(
            player->rigid_bodies,
            player->alive_body_id,
            vec(0.0f, -PLAYER_JUMP));
        player->jump_threshold++;

        if (script_has_scope_value(player->script, "on-jump")) {
            script_eval(player->script, "(on-jump)");
        }
    }
}

void player_die(Player *player)
{
    trace_assert(player);

    if (player->state == PLAYER_STATE_ALIVE) {
        const Rect hitbox =
            rigid_bodies_hitbox(
                player->rigid_bodies,
                player->alive_body_id);

        player->play_die_cue = 1;
        dying_rect_start_dying(player->dying_body, vec(hitbox.x, hitbox.y));
        player->state = PLAYER_STATE_DYING;
    }
}

void player_focus_camera(Player *player,
                         Camera *camera)
{
    trace_assert(player);
    trace_assert(camera);

    const Rect player_hitbox = rigid_bodies_hitbox(
        player->rigid_bodies,
        player->alive_body_id);

    camera_center_at(
        camera,
        vec_sum(
            vec(player_hitbox.x, player_hitbox.y),
            vec(0.0f, -player_hitbox.h * 0.5f)));
}

void player_hide_goals(const Player *player,
                       Goals *goals)
{
    trace_assert(player);
    trace_assert(goals);
    goals_hide_from_player(
        goals,
        rigid_bodies_hitbox(
            player->rigid_bodies,
            player->alive_body_id));
}

void player_die_from_lava(Player *player,
                          const Lava *lava)
{
    if (lava_overlaps_rect(
            lava,
            rigid_bodies_hitbox(
                player->rigid_bodies,
                player->alive_body_id))) {
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

bool player_overlaps_rect(const Player *player,
                          Rect rect)
{
    trace_assert(player);

    return player->state == PLAYER_STATE_ALIVE
        && rects_overlap(
            rect, rigid_bodies_hitbox(
                player->rigid_bodies,
                player->alive_body_id));
}
