#include <SDL2/SDL.h>
#include "system/stacktrace.h"

#include "broadcast.h"
#include "color.h"
#include "ebisp/builtins.h"
#include "ebisp/interpreter.h"
#include "game/camera.h"
#include "game/level.h"
#include "game/level/background.h"
#include "game/level/boxes.h"
#include "game/level/goals.h"
#include "game/level/labels.h"
#include "game/level/lava.h"
#include "game/level/physical_world.h"
#include "game/level/platforms.h"
#include "game/level/player.h"
#include "game/level/regions.h"
#include "game/level/rigid_bodies.h"
#include "system/line_stream.h"
#include "system/lt.h"
#include "system/lt/lt_adapters.h"
#include "system/nth_alloc.h"

#define LEVEL_LINE_MAX_LENGTH 512
#define LEVEL_GRAVITY 1500.0f

struct Level
{
    Lt *lt;

    Background *background;
    Player *player;
    Platforms *platforms;
    Goals *goals;
    Lava *lava;
    Platforms *back_platforms;
    Boxes *boxes;
    Labels *labels;
    Regions *regions;
    Physical_world *physical_world;
    RigidBodies *rigid_bodies;
};

Level *create_level_from_file(const char *file_name, Broadcast *broadcast)
{
    trace_assert(file_name);

    Lt *const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Level *const level = PUSH_LT(lt, nth_alloc(sizeof(Level)), free);
    if (level == NULL) {
        RETURN_LT(lt, NULL);
    }

    LineStream *level_stream = PUSH_LT(
        lt,
        create_line_stream(
            file_name,
            "r",
            LEVEL_LINE_MAX_LENGTH),
        destroy_line_stream);
    if (level_stream == NULL) {
        RETURN_LT(lt, NULL);
    }
    level->background = PUSH_LT(
        lt,
        create_background_from_line_stream(level_stream),
        destroy_background);
    if (level->background == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->player = PUSH_LT(
        lt,
        create_player_from_line_stream(level_stream, broadcast),
        destroy_player);
    if (level->player == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->platforms = PUSH_LT(
        lt,
        create_platforms_from_line_stream(level_stream),
        destroy_platforms);
    if (level->platforms == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->goals = PUSH_LT(
        lt,
        create_goals_from_line_stream(level_stream),
        destroy_goals);
    if (level->goals == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->lava = PUSH_LT(
        lt,
        create_lava_from_line_stream(level_stream),
        destroy_lava);
    if (level->lava == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->back_platforms = PUSH_LT(
        lt,
        create_platforms_from_line_stream(level_stream),
        destroy_platforms);
    if (level->back_platforms == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->boxes = PUSH_LT(
        lt,
        create_boxes_from_line_stream(level_stream),
        destroy_boxes);
    if (level->boxes == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->labels = PUSH_LT(
        lt,
        create_labels_from_line_stream(level_stream),
        destroy_labels);
    if (level->labels == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->regions = PUSH_LT(
        lt,
        create_regions_from_line_stream(level_stream, broadcast),
        destroy_regions);
    if (level->regions == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->physical_world = PUSH_LT(lt, create_physical_world(), destroy_physical_world);
    if (level->physical_world == NULL) {
        RETURN_LT(lt, NULL);
    }
    if (physical_world_add_solid(
            level->physical_world,
            player_as_solid(level->player)) < 0) { RETURN_LT(lt, NULL); }
    if (boxes_add_to_physical_world(
            level->boxes,
            level->physical_world) < 0) { RETURN_LT(lt, NULL); }

    level->rigid_bodies = PUSH_LT(lt, create_rigid_bodies(1024), destroy_rigid_bodies);
    if (level->rigid_bodies == NULL) {
        RETURN_LT(lt, NULL);
    }
    rigid_bodies_add(
        level->rigid_bodies,
        rect(0.0, -1000.0, 100.0, 100.0),
        rgba(1.0f, 0.0f, 0.0f, 1.0f));

    rigid_bodies_add(
        level->rigid_bodies,
        rect(150.0, -1000.0, 100.0, 100.0),
        rgba(0.0f, 1.0f, 0.0f, 1.0f));


    level->lt = lt;

    destroy_line_stream(RELEASE_LT(lt, level_stream));

    return level;
}

void destroy_level(Level *level)
{
    trace_assert(level);
    RETURN_LT0(level->lt);
}

int level_render(const Level *level, Camera *camera)
{
    trace_assert(level);

    if (background_render(level->background, camera) < 0) {
        return -1;
    }

    if (platforms_render(level->back_platforms, camera) < 0) {
        return -1;
    }

    if (player_render(level->player, camera) < 0) {
        return -1;
    }

    if (boxes_render(level->boxes, camera) < 0) {
        return -1;
    }

    if (lava_render(level->lava, camera) < 0) {
        return -1;
    }

    if (platforms_render(level->platforms, camera) < 0) {
        return -1;
    }

    if (goals_render(level->goals, camera) < 0) {
        return -1;
    }

    if (labels_render(level->labels, camera) < 0) {
        return -1;
    }

    if (regions_render(level->regions, camera) < 0) {
        return -1;
    }

    if (rigid_bodies_render(level->rigid_bodies, camera) < 0) {
        return -1;
    }

    return 0;
}

int level_update(Level *level, float delta_time)
{
    trace_assert(level);
    trace_assert(delta_time > 0);

    physical_world_apply_gravity(level->physical_world);
    boxes_float_in_lava(level->boxes, level->lava);
    rigid_bodies_apply_omniforce(level->rigid_bodies, vec(0.0f, LEVEL_GRAVITY));

    rigid_bodies_update(level->rigid_bodies, delta_time);
    boxes_update(level->boxes, delta_time);
    player_update(level->player, delta_time);

    rigid_bodies_collide(level->rigid_bodies, level->platforms);
    physical_world_collide_solids(level->physical_world, level->platforms);

    player_hide_goals(level->player, level->goals);
    player_die_from_lava(level->player, level->lava);
    regions_player_enter(level->regions, level->player);
    regions_player_leave(level->regions, level->player);

    goals_update(level->goals, delta_time);
    lava_update(level->lava, delta_time);
    labels_update(level->labels, delta_time);

    return 0;
}

int level_event(Level *level, const SDL_Event *event)
{
    trace_assert(level);
    trace_assert(event);

    switch (event->type) {
    case SDL_KEYDOWN:
        switch (event->key.keysym.sym) {
        case SDLK_SPACE:
            player_jump(level->player);
            break;
        }
        break;

    case SDL_JOYBUTTONDOWN:
        if (event->jbutton.button == 1) {
            player_jump(level->player);
        }
        break;
    }

    return 0;
}

int level_input(Level *level,
                const Uint8 *const keyboard_state,
                SDL_Joystick *the_stick_of_joy)
{
    trace_assert(level);
    trace_assert(keyboard_state);
    (void) the_stick_of_joy;

    if (keyboard_state[SDL_SCANCODE_A]) {
        player_move_left(level->player);
    } else if (keyboard_state[SDL_SCANCODE_D]) {
        player_move_right(level->player);
    } else if (the_stick_of_joy && SDL_JoystickGetAxis(the_stick_of_joy, 0) < 0) {
        player_move_left(level->player);
    } else if (the_stick_of_joy && SDL_JoystickGetAxis(the_stick_of_joy, 0) > 0) {
        player_move_right(level->player);
    } else {
        player_stop(level->player);
    }

    return 0;
}

int level_reload_preserve_player(Level *level, const char *file_name, Broadcast *broadcast)
{
    Lt * const lt = create_lt();
    if (lt == NULL) {
        return -1;
    }

    /* TODO(#104): duplicate code in create_level_from_file and level_reload_preserve_player */

    LineStream * const level_stream = PUSH_LT(
        lt,
        create_line_stream(
            file_name,
            "r",
            LEVEL_LINE_MAX_LENGTH),
        destroy_line_stream);
    if (level_stream == NULL) {
        RETURN_LT(lt, -1);
    }

    Background * const background = create_background_from_line_stream(level_stream);
    if (background == NULL) {
        RETURN_LT(lt, -1);
    }
    level->background = RESET_LT(level->lt, level->background, background);

    Player * const skipped_player = create_player_from_line_stream(level_stream, broadcast);
    if (skipped_player == NULL) {
        RETURN_LT(lt, -1);
    }
    destroy_player(skipped_player);

    Platforms * const platforms = create_platforms_from_line_stream(level_stream);
    if (platforms == NULL) {
        RETURN_LT(lt, -1);
    }
    level->platforms = RESET_LT(level->lt, level->platforms, platforms);

    Goals * const goals = create_goals_from_line_stream(level_stream);
    if (goals == NULL) {
        RETURN_LT(lt, -1);
    }
    level->goals = RESET_LT(level->lt, level->goals, goals);

    Lava * const lava = create_lava_from_line_stream(level_stream);
    if (lava == NULL) {
        RETURN_LT(lt, -1);
    }
    level->lava = RESET_LT(level->lt, level->lava, lava);

    Platforms * const back_platforms = create_platforms_from_line_stream(level_stream);
    if (back_platforms == NULL) {
        RETURN_LT(lt, -1);
    }
    level->back_platforms = RESET_LT(level->lt, level->back_platforms, back_platforms);

    Boxes * const boxes = create_boxes_from_line_stream(level_stream);
    if (boxes == NULL) {
        RETURN_LT(lt, -1);
    }
    level->boxes = RESET_LT(level->lt, level->boxes, boxes);

    Labels * const labels = create_labels_from_line_stream(level_stream);
    if (labels == NULL) {
        RETURN_LT(lt, -1);
    }
    level->labels = RESET_LT(level->lt, level->labels, labels);

    Regions * const regions = create_regions_from_line_stream(level_stream, broadcast);
    if (regions == NULL) {
        RETURN_LT(lt, -1);
    }
    level->regions = RESET_LT(level->lt, level->regions, regions);

    physical_world_clean(level->physical_world);
    if (physical_world_add_solid(
            level->physical_world,
            player_as_solid(level->player)) < 0) { RETURN_LT(lt, -1); }
    if (boxes_add_to_physical_world(
            level->boxes,
            level->physical_world) < 0) { RETURN_LT(lt, -1); }

    RETURN_LT(lt, 0);
}

int level_sound(Level *level, Sound_samples *sound_samples)
{
    if (goals_sound(level->goals, sound_samples) < 0) {
        return -1;
    }

    if (player_sound(level->player, sound_samples) < 0) {
        return -1;
    }

    return 0;
}

void level_toggle_debug_mode(Level *level)
{
    background_toggle_debug_mode(level->background);
}

int level_enter_camera_event(Level *level, Camera *camera)
{
    player_focus_camera(level->player, camera);
    goals_cue(level->goals, camera);
    goals_checkpoint(level->goals, level->player);
    labels_enter_camera_event(level->labels, camera);
    return 0;
}

Rigid_rect *level_rigid_rect(Level *level,
                             const char *rigid_rect_id)
{
    trace_assert(level);
    trace_assert(rigid_rect_id);

    Rigid_rect *rigid_rect = player_rigid_rect(level->player,
                                               rigid_rect_id);
    if (rigid_rect != NULL) {
        return rigid_rect;
    }

    rigid_rect = boxes_rigid_rect(level->boxes, rigid_rect_id);
    if (rigid_rect != NULL) {
        return rigid_rect;
    }

    return NULL;
}

struct EvalResult level_send(Level *level, Gc *gc, struct Scope *scope, struct Expr path)
{
    trace_assert(level);
    trace_assert(gc);
    trace_assert(scope);

    const char *target = NULL;
    struct Expr rest = void_expr();
    struct EvalResult res = match_list(gc, "q*", path, &target, &rest);
    if (res.is_error) {
        return res;
    }

    if (strcmp(target, "goal") == 0) {
        return goals_send(level->goals, gc, scope, rest);
    } else if (strcmp(target, "label") == 0) {
        return labels_send(level->labels, gc, scope, rest);
    } else if (strcmp(target, "box") == 0) {
        return boxes_send(level->boxes, gc, scope, rest);
    } else if (strcmp(target, "body-push") == 0) {
        long int id = 0, x = 0, y = 0;
        res = match_list(gc, "ddd", rest, &id, &x, &y);
        if (res.is_error) {
            return res;
        }

        rigid_bodies_apply_force(level->rigid_bodies, (size_t) id, vec((float) x, (float) y));

        return eval_success(NIL(gc));
    } else if (strcmp(target, "body-add") == 0) {
        long int x = 0, y = 0, w = 0, h = 0;
        const char *color = 0;
        res = match_list(gc, "dddds", rest, &x, &y, &w, &h, &color);
        if (res.is_error) {
            return res;
        }

        return eval_success(
            NUMBER(
                gc,
                (long int) rigid_bodies_add(
                    level->rigid_bodies,
                    rect((float)x, (float)y, (float)w, (float)h),
                    hexstr(color))));
    }

    return unknown_target(gc, "level", target);
}
