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
#include "game/level/platforms.h"
#include "game/level/player.h"
#include "game/level/regions.h"
#include "game/level/rigid_bodies.h"
#include "game/level_metadata.h"
#include "game/level/level_editor/proto_rect.h"
#include "game/level/level_editor/layer.h"
#include "game/level/level_editor/point_layer.h"
#include "system/line_stream.h"
#include "system/log.h"
#include "system/lt.h"
#include "system/lt/lt_adapters.h"
#include "system/nth_alloc.h"
#include "system/str.h"
#include "game/level/level_editor.h"

#define LEVEL_LINE_MAX_LENGTH 512
#define LEVEL_GRAVITY 1500.0f

struct Level
{
    Lt *lt;

    const char *file_name;
    LevelMetadata *metadata;
    // TODO(#812): LevelEditor does not support Background
    Background *background;
    RigidBodies *rigid_bodies;
    // TODO(#813): LevelEditor does not support Player
    Player *player;
    Platforms *platforms;
    Goals *goals;
    // TODO(#816): LevelEditor does not support Lava
    Lava *lava;
    Platforms *back_platforms;
    Boxes *boxes;
    // TODO(#818): LevelEditor does not support Labels
    Labels *labels;
    // TODO(#819): LevelEditor does not support Regions
    Regions *regions;

    bool edit_mode;
    LevelEditor *level_editor;
};

Level *create_level_from_file(const char *file_name, Broadcast *broadcast)
{
    trace_assert(file_name);

    Lt *const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Level *const level = PUSH_LT(lt, nth_calloc(1, sizeof(Level)), free);
    if (level == NULL) {
        RETURN_LT(lt, NULL);
    }
    level->lt = lt;

    level->file_name = PUSH_LT(lt, string_duplicate(file_name, NULL), free);
    if (level->file_name == NULL) {
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

    level->metadata = PUSH_LT(
        lt,
        create_level_metadata_from_line_stream(level_stream),
        destroy_level_metadata);
    if (level->metadata == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->background = PUSH_LT(
        lt,
        create_background_from_line_stream(level_stream),
        destroy_background);
    if (level->background == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->rigid_bodies = PUSH_LT(lt, create_rigid_bodies(1024), destroy_rigid_bodies);
    if (level->rigid_bodies == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->player = PUSH_LT(
        lt,
        create_player_from_line_stream(level_stream, level->rigid_bodies, broadcast),
        destroy_player);
    if (level->player == NULL) {
        RETURN_LT(lt, NULL);
    }

    RectLayer *platforms_rect_layer = create_layer_from_line_stream(level_stream);
    if (platforms_rect_layer == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->platforms = PUSH_LT(
        lt,
        create_platforms_from_rect_layer(platforms_rect_layer),
        destroy_platforms);
    if (level->platforms == NULL) {
        RETURN_LT(lt, NULL);
    }

    PointLayer *goals_rect_layer = create_point_layer_from_line_stream(level_stream);
    if (goals_rect_layer == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->goals = PUSH_LT(
        lt,
        create_goals_from_point_layer(goals_rect_layer),
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

    RectLayer *back_platforms_rect_layer = create_layer_from_line_stream(level_stream);
    if (back_platforms_rect_layer == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->back_platforms = PUSH_LT(
        lt,
        create_platforms_from_rect_layer(back_platforms_rect_layer),
        destroy_platforms);
    if (level->back_platforms == NULL) {
        RETURN_LT(lt, NULL);
    }

    RectLayer *boxes_rect_layer = create_layer_from_line_stream(level_stream);
    if (boxes_rect_layer == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->boxes = PUSH_LT(
        lt,
        create_boxes_from_rect_layer(boxes_rect_layer, level->rigid_bodies),
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

    level->edit_mode = false;
    level->level_editor = PUSH_LT(
        lt,
        create_level_editor(
            boxes_rect_layer,
            platforms_rect_layer,
            back_platforms_rect_layer,
            goals_rect_layer),
        destroy_level_editor);
    if (level->level_editor == NULL) {
        RETURN_LT(lt, NULL);
    }

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

    if (level->edit_mode) {
        if (level_editor_render(level->level_editor, camera) < 0) {
            return -1;
        }

        return 0;
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

    return 0;
}

int level_update(Level *level, float delta_time)
{
    trace_assert(level);
    trace_assert(delta_time > 0);

    boxes_float_in_lava(level->boxes, level->lava);
    rigid_bodies_apply_omniforce(level->rigid_bodies, vec(0.0f, LEVEL_GRAVITY));

    boxes_update(level->boxes, delta_time);
    player_update(level->player, delta_time);

    rigid_bodies_collide(level->rigid_bodies, level->platforms);

    player_hide_goals(level->player, level->goals);
    player_die_from_lava(level->player, level->lava);
    regions_player_enter(level->regions, level->player);
    regions_player_leave(level->regions, level->player);

    goals_update(level->goals, delta_time);
    lava_update(level->lava, delta_time);
    labels_update(level->labels, delta_time);

    if (level->edit_mode) {
        level_editor_update(level->level_editor, delta_time);
    }

    return 0;
}

int level_event(Level *level, const SDL_Event *event, const Camera *camera)
{
    trace_assert(level);
    trace_assert(event);

    switch (event->type) {
    case SDL_KEYDOWN:
        switch (event->key.keysym.sym) {
        case SDLK_SPACE: {
            player_jump(level->player);
        } break;

        case SDLK_TAB: {
            level->edit_mode = !level->edit_mode;
            SDL_SetRelativeMouseMode(level->edit_mode);
            if (!level->edit_mode) {
                level->boxes = RESET_LT(
                    level->lt,
                    level->boxes,
                    create_boxes_from_rect_layer(
                        level_editor_boxes(level->level_editor),
                        level->rigid_bodies));
                if (level->boxes == NULL) {
                    return -1;
                }

                level->platforms = RESET_LT(
                    level->lt,
                    level->platforms,
                    create_platforms_from_rect_layer(
                        level_editor_platforms(
                            level->level_editor)));
                if (level->platforms == NULL) {
                    return -1;
                }

                level->back_platforms = RESET_LT(
                    level->lt,
                    level->back_platforms,
                    create_platforms_from_rect_layer(
                        level_editor_back_platforms(
                            level->level_editor)));
                if (level->back_platforms == NULL) {
                    return -1;
                }

                // TODO(#834): goals are not updated after tabbing from LevelEditor
            }
        };
        }
        break;

    case SDL_JOYBUTTONDOWN:
        if (event->jbutton.button == 1) {
            player_jump(level->player);
        }
        break;
    }

    if (level->edit_mode) {
        level_editor_event(level->level_editor, event, camera);
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
    if (!level->edit_mode) {
        player_focus_camera(level->player, camera);
        camera_scale(camera, 1.0f);
    } else {
        level_editor_focus_camera(
            level->level_editor,
            camera);
    }

    goals_cue(level->goals, camera);
    goals_checkpoint(level->goals, level->player);
    labels_enter_camera_event(level->labels, camera);
    return 0;
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
    } else if (strcmp(target, "edit") == 0) {
        level->edit_mode = !level->edit_mode;
        SDL_SetRelativeMouseMode(level->edit_mode);
        return eval_success(NIL(gc));
    }

    return unknown_target(gc, "level", target);
}

bool level_edit_mode(const Level *level)
{
    trace_assert(level);
    return level->edit_mode;
}
