#include <SDL2/SDL.h>
#include <assert.h>

#include "color.h"
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
#include "system/error.h"
#include "system/lt.h"
#include "system/lt/lt_adapters.h"

struct level_t
{
    lt_t *lt;

    physical_world_t *physical_world;
    player_t *player;
    platforms_t *platforms;
    goals_t *goals;
    lava_t *lava;
    color_t background_color;
    platforms_t *back_platforms;
    background_t *background;
    boxes_t *boxes;
    labels_t *labels;
};

level_t *create_level_from_file(const char *file_name)
{
    assert(file_name);

    lt_t *const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    level_t *const level = PUSH_LT(lt, malloc(sizeof(level_t)), free);
    if (level == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    FILE *level_file = PUSH_LT(lt, fopen(file_name, "r"), fclose_lt);
    if (level_file == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    char color[7];
    if (fscanf(level_file, "%6s", color) == EOF) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    level->background_color = color_from_hexstr(color);

    level->player = PUSH_LT(lt, create_player_from_stream(level_file), destroy_player);
    if (level->player == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->platforms = PUSH_LT(lt, create_platforms_from_stream(level_file), destroy_platforms);
    if (level->platforms == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->goals = PUSH_LT(lt, create_goals_from_stream(level_file), destroy_goals);
    if (level->goals == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->lava = PUSH_LT(lt, create_lava_from_stream(level_file), destroy_lava);
    if (level->lava == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->back_platforms = PUSH_LT(lt, create_platforms_from_stream(level_file), destroy_platforms);
    if (level->back_platforms == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->boxes = PUSH_LT(lt, create_boxes_from_stream(level_file), destroy_boxes);
    if (level->boxes == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->labels = PUSH_LT(lt, create_labels_from_stream(level_file), destroy_labels);
    if (level->labels == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->background = PUSH_LT(lt, create_background(level->background_color), destroy_background);
    if (level->background == NULL) {
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

    level->lt = lt;

    fclose(RELEASE_LT(lt, level_file));

    return level;
}

void destroy_level(level_t *level)
{
    assert(level);
    RETURN_LT0(level->lt);
}

int level_render(const level_t *level, camera_t *camera)
{
    assert(level);

    player_focus_camera(level->player, camera);

    if (camera_clear_background(camera, level->background_color) < 0) {
        return -1;
    }

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

    return 0;
}

int level_update(level_t *level, float delta_time)
{
    assert(level);
    assert(delta_time > 0);

    physical_world_apply_gravity(level->physical_world);

    boxes_update(level->boxes, delta_time);
    player_update(level->player, delta_time);

    physical_world_collide_solids(level->physical_world, level->platforms);

    player_hide_goals(level->player, level->goals);
    player_die_from_lava(level->player, level->lava);

    goals_update(level->goals, delta_time);
    lava_update(level->lava, delta_time);
    labels_update(level->labels, delta_time);

    return 0;
}

int level_event(level_t *level, const SDL_Event *event)
{
    assert(level);
    assert(event);

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

int level_input(level_t *level,
                const Uint8 *const keyboard_state,
                SDL_Joystick *the_stick_of_joy)
{
    assert(level);
    assert(keyboard_state);
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

int level_reload_preserve_player(level_t *level, const char *file_name)
{
    lt_t * const lt = create_lt();
    if (lt == NULL) {
        return -1;
    }

    /* TODO(#104): duplicate code in create_level_from_file and level_reload_preserve_player */



    FILE * const level_file = PUSH_LT(lt, fopen(file_name, "r"), fclose_lt);
    if (level_file == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, -1);
    }

    char color[7];
    if (fscanf(level_file, "%6s", color) == EOF) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, -1);
    }
    level->background_color = color_from_hexstr(color);

    player_t * const skipped_player = create_player_from_stream(level_file);
    if (skipped_player == NULL) {
        RETURN_LT(lt, -1);
    }
    destroy_player(skipped_player);

    platforms_t * const platforms = create_platforms_from_stream(level_file);
    if (platforms == NULL) {
        RETURN_LT(lt, -1);
    }
    level->platforms = RESET_LT(level->lt, level->platforms, platforms);

    goals_t * const goals = create_goals_from_stream(level_file);
    if (goals == NULL) {
        RETURN_LT(lt, -1);
    }
    level->goals = RESET_LT(level->lt, level->goals, goals);

    lava_t * const lava = create_lava_from_stream(level_file);
    if (lava == NULL) {
        RETURN_LT(lt, -1);
    }
    level->lava = RESET_LT(level->lt, level->lava, lava);

    platforms_t * const back_platforms = create_platforms_from_stream(level_file);
    if (back_platforms == NULL) {
        RETURN_LT(lt, -1);
    }
    level->back_platforms = RESET_LT(level->lt, level->back_platforms, back_platforms);

    boxes_t * const boxes = create_boxes_from_stream(level_file);
    if (boxes == NULL) {
        RETURN_LT(lt, -1);
    }
    level->boxes = RESET_LT(level->lt, level->boxes, boxes);

    labels_t * const labels = create_labels_from_stream(level_file);
    if (labels == NULL) {
        RETURN_LT(lt, -1);
    }
    level->labels = RESET_LT(level->lt, level->labels, labels);

    physical_world_clean(level->physical_world);
    if (physical_world_add_solid(
            level->physical_world,
            player_as_solid(level->player)) < 0) { RETURN_LT(lt, -1); }
    if (boxes_add_to_physical_world(
            level->boxes,
            level->physical_world) < 0) { RETURN_LT(lt, -1); }

    RETURN_LT(lt, 0);
}

int level_sound(level_t *level, sound_samples_t *sound_samples)
{
    if (goals_sound(level->goals, sound_samples) < 0) {
        return -1;
    }

    if (player_sound(level->player, sound_samples) < 0) {
        return -1;
    }

    return 0;
}

void level_toggle_debug_mode(level_t *level)
{
    background_toggle_debug_mode(level->background);
}

int level_enter_camera_event(level_t *level,
                             const camera_t *camera)
{
    goals_cue(level->goals, camera);
    goals_checkpoint(level->goals, level->player);
    labels_enter_camera_event(level->labels, camera);
    return 0;
}
