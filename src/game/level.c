#include <SDL.h>
#include "system/stacktrace.h"

#include "color.h"
#include "game/camera.h"
#include "game/level.h"
#include "game/level/background.h"
#include "game/level/boxes.h"
#include "game/level/goals.h"
#include "game/level/labels.h"
#include "game/level/lava.h"
#include "game/level/platforms.h"
#include "game/level/phantom_platforms.h"
#include "game/level/player.h"
#include "game/level/regions.h"
#include "game/level/rigid_bodies.h"
#include "game/level/level_editor/rect_layer.h"
#include "game/level/level_editor/point_layer.h"
#include "game/level/level_editor/player_layer.h"
#include "game/level/level_editor/label_layer.h"
#include "game/level/level_editor/background_layer.h"
#include "system/log.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/str.h"
#include "game/level/level_editor.h"
#include "ui/console.h"

#define LEVEL_GRAVITY 1500.0f
#define JOYSTICK_THRESHOLD 1000

typedef enum {
    LEVEL_STATE_IDLE = 0,
    LEVEL_STATE_PAUSE
} LevelState;

#define PLATFORM_LAYERS_COUNT 2
#define PLATFORM_LAYERS_SPEED 8.0f

struct Level
{
    Lt *lt;

    LevelState state;
    Background background;
    RigidBodies *rigid_bodies;
    Player *player;
    Platforms *platforms;
    Goals *goals;
    Lava *lava;
    Platforms *back_platforms;
    Boxes *boxes;
    Labels *labels;
    Regions *regions;
    Phantom_Platforms pp;

    Platforms *platform_layers[PLATFORM_LAYERS_COUNT];
    size_t current_platforms;
    size_t prev_platform;

    float a;                    // -1.0 <= a <= 1.0
                                // abs(a) == 1.0
    float da;
};

Level *create_level_from_level_editor(const LevelEditor *level_editor)
{
    trace_assert(level_editor);

    Lt *lt = create_lt();

    Level *level = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(Level)),
        free);
    if (level == NULL) {
        RETURN_LT(lt, NULL);
    }
    level->lt = lt;

    level->background = create_background(
        color_picker_rgba(
            &level_editor->background_layer.color_picker));

    level->rigid_bodies = PUSH_LT(lt, create_rigid_bodies(1024), destroy_rigid_bodies);
    if (level->rigid_bodies == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->player = PUSH_LT(
        lt,
        create_player_from_player_layer(
            &level_editor->player_layer,
            level->rigid_bodies),
        destroy_player);
    if (level->player == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->platforms = PUSH_LT(
        lt,
        create_platforms_from_rect_layer(level_editor->platforms_layer),
        destroy_platforms);
    if (level->platforms == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->goals = PUSH_LT(
        lt,
        create_goals_from_point_layer(level_editor->goals_layer),
        destroy_goals);
    if (level->goals == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->lava = PUSH_LT(
        lt,
        create_lava_from_rect_layer(level_editor->lava_layer),
        destroy_lava);
    if (level->lava == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->back_platforms = PUSH_LT(
        lt,
        create_platforms_from_rect_layer(level_editor->back_platforms_layer),
        destroy_platforms);
    if (level->back_platforms == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->boxes = PUSH_LT(
        lt,
        create_boxes_from_rect_layer(level_editor->boxes_layer, level->rigid_bodies),
        destroy_boxes);
    if (level->boxes == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->labels = PUSH_LT(
        lt,
        create_labels_from_label_layer(level_editor->label_layer),
        destroy_labels);
    if (level->labels == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->regions = PUSH_LT(
        lt,
        create_regions_from_rect_layer(
            level_editor->regions_layer,
            level->labels,
            level->goals),
        destroy_regions);
    if (level->regions == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->pp = create_phantom_platforms(level_editor->pp_layer);

    level->platform_layers[0] = level->back_platforms;
    level->platform_layers[1] = level->platforms;
    level->current_platforms = 1;

    return level;
}

void destroy_level(Level *level)
{
    trace_assert(level);
    destroy_phantom_platforms(level->pp);
    RETURN_LT0(level->lt);
}

int level_render(const Level *level, const Camera *camera)
{
    trace_assert(level);

    if (background_render(&level->background, camera) < 0) {
        return -1;
    }

    // d could-be-= 1.0 / N, but lets assume d = 0.1
    //
    //  0     1     2     3     4
    //              ^
    // 0.8   0.9   1.0   1.1   1.2
    //       ^
    // 0.9   1.0   1.1   1.2   1.3
    // 1.0 + (i - current) * 0.1

    for (size_t i = 0; i < PLATFORM_LAYERS_COUNT; ++i) {
        const float d = 0.2f;
        Camera camera0 = *camera;

        if (level->da == 0.0f) {
            camera0.scale =
                fmaxf(0.0f, 1.0f + ((float) i - (float) level->current_platforms) * d);
        } else {
            camera0.scale =
                fmaxf(0.0f, 1.0f + ((float) i - (float) level->prev_platform + level->a) * d);
        }


        if (i != level->current_platforms) {
            camera0.transparent_mode = 1;
        }

        platforms_render(level->platform_layers[i], &camera0);

        if (i == level->current_platforms) {
            if (player_render(level->player, camera) < 0) {
                return -1;
            }
        }
    }

    phantom_platforms_render(&level->pp, camera);


    if (boxes_render(level->boxes, camera) < 0) {
        return -1;
    }

    if (lava_render(level->lava, camera) < 0) {
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

    return 0;
}

int level_update(Level *level, float delta_time)
{
    trace_assert(level);
    trace_assert(delta_time > 0);

    if (level->state == LEVEL_STATE_PAUSE) {
        return 0;
    }

    boxes_float_in_lava(level->boxes, level->lava);
    rigid_bodies_apply_omniforce(level->rigid_bodies, vec(0.0f, LEVEL_GRAVITY));

    boxes_update(level->boxes, delta_time);
    player_update(level->player, delta_time);

    rigid_bodies_collide(level->rigid_bodies,
                         level->platform_layers[level->current_platforms]);

    player_die_from_lava(level->player, level->lava);
    regions_player_enter(level->regions, level->player);
    regions_player_leave(level->regions, level->player);

    goals_update(level->goals, delta_time);
    lava_update(level->lava, delta_time);
    labels_update(level->labels, delta_time);

    Rect hitbox = player_hitbox(level->player);
    phantom_platforms_hide_at(&level->pp, vec(hitbox.x, hitbox.y));
    phantom_platforms_update(&level->pp, delta_time);

    if (level->da != 0.0f) {
        level->a += level->da * delta_time;
        if (fabsf(level->a) > 1.0f) {
            level->da = 0.0f;
            level->a = 0.0f;
        }
    }

    return 0;
}

static
int level_event_idle(Level *level, const SDL_Event *event,
                     Camera *camera, Sound_samples *sound_samples)
{
    trace_assert(level);

    switch (event->type) {
    case SDL_KEYDOWN:
        switch (event->key.keysym.sym) {
        case SDLK_w:
        case SDLK_UP:
        case SDLK_SPACE: {
            player_jump(level->player);
        } break;

        case SDLK_p: {
            level->state = LEVEL_STATE_PAUSE;
            camera->blackwhite_mode = true;
            sound_samples_toggle_pause(sound_samples);
        } break;

        case SDLK_l: {
            camera_toggle_debug_mode(camera);
        } break;

        case SDLK_1: {
            if (level->current_platforms > 0) {
                level->a = 0.0f;
                level->da = PLATFORM_LAYERS_SPEED;
                level->prev_platform = level->current_platforms;
                level->current_platforms -= 1;
            }
        } break;

        case SDLK_2: {
            if (level->current_platforms + 1 < PLATFORM_LAYERS_COUNT) {
                level->a = 0.0f;
                level->da = -PLATFORM_LAYERS_SPEED;
                level->prev_platform = level->current_platforms;
                level->current_platforms += 1;
            }
        } break;
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

static
int level_event_pause(Level *level, const SDL_Event *event,
                      Camera *camera, Sound_samples *sound_samples)
{
    trace_assert(level);

    switch (event->type) {
    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_p: {
            level->state = LEVEL_STATE_IDLE;
            camera->blackwhite_mode = false;
            sound_samples_toggle_pause(sound_samples);
        } break;
        }
    } break;
    }

    return 0;
}

int level_event(Level *level, const SDL_Event *event,
                Camera *camera, Sound_samples *sound_samples)
{
    trace_assert(level);
    trace_assert(event);

    switch (level->state) {
    case LEVEL_STATE_IDLE: {
        return level_event_idle(level, event, camera, sound_samples);
    } break;

    case LEVEL_STATE_PAUSE: {
        return level_event_pause(level, event, camera, sound_samples);
    } break;
    }

    return 0;
}

int level_input(Level *level,
                const Uint8 *const keyboard_state,
                SDL_Joystick *the_stick_of_joy)
{
    trace_assert(level);
    trace_assert(keyboard_state);

    if (level->state == LEVEL_STATE_PAUSE) {
        return 0;
    }

    if (keyboard_state[SDL_SCANCODE_A] || keyboard_state[SDL_SCANCODE_LEFT]) {
        player_move_left(level->player);
    } else if (keyboard_state[SDL_SCANCODE_D] || keyboard_state[SDL_SCANCODE_RIGHT]) {
        player_move_right(level->player);
    } else if (the_stick_of_joy && SDL_JoystickGetAxis(the_stick_of_joy, 0) < -JOYSTICK_THRESHOLD) {
        player_move_left(level->player);
    } else if (the_stick_of_joy && SDL_JoystickGetAxis(the_stick_of_joy, 0) > JOYSTICK_THRESHOLD) {
        player_move_right(level->player);
    } else {
        player_stop(level->player);
    }

    return 0;
}

int level_sound(Level *level, Sound_samples *sound_samples)
{
    if (level->state == LEVEL_STATE_PAUSE) {
        return 0;
    }

    if (goals_sound(level->goals, sound_samples) < 0) {
        return -1;
    }

    if (player_sound(level->player, sound_samples) < 0) {
        return -1;
    }

    return 0;
}

int level_enter_camera_event(Level *level, Camera *camera)
{
    if (level->state == LEVEL_STATE_PAUSE) {
        return 0;
    }

    player_focus_camera(level->player, camera);
    camera_scale(camera, 1.0f);

    goals_cue(level->goals, camera);
    goals_checkpoint(level->goals, level->player);
    labels_enter_camera_event(level->labels, camera);
    return 0;
}

void level_disable_pause_mode(Level *level, Camera *camera,
                              Sound_samples *sound_samples)
{
    trace_assert(level);
    trace_assert(camera);
    trace_assert(sound_samples);
    level->state = LEVEL_STATE_IDLE;
    camera->blackwhite_mode = false;
    sound_samples_toggle_pause(sound_samples);
}
