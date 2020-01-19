#include <SDL.h>

#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "game/level/platforms.h"
#include "game/level/player.h"
#include "game/sound_samples.h"
#include "game/sprite_font.h"
#include "math/extrema.h"
#include "math/vec.h"
#include "sdl/renderer.h"
#include "system/log.h"
#include "system/lt.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

static void print_usage(FILE *stream)
{
    fprintf(stream, "Usage: nothing [--fps <fps>]\n");
}

static float current_display_scale = 1.0f;


// export this for other parts of the code to use.
float get_display_scale(void)
{
    return current_display_scale;
}

static
void recalculate_display_scale(SDL_Window* win, SDL_Renderer* rend)
{
    int w0 = 0;
    SDL_GetWindowSize(win, &w0, NULL);

    int w1 = 0;
    SDL_GetRendererOutputSize(rend, &w1, NULL);

    current_display_scale = (float) w1 / (float) w0;
}

static
void maybe_fixup_input_for_display_scale(SDL_Window* win, SDL_Renderer* rend, SDL_Event* e)
{
    // note: we check for window move as well, because you may move the window to
    // another monitor with a different display scale.
    switch (e->type) {
    case SDL_WINDOWEVENT: {
        switch (e->window.event) {
        case SDL_WINDOWEVENT_MOVED:
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            recalculate_display_scale(win, rend);
            break;
        }
    } break;

    // this is the fixup.
    case SDL_MOUSEMOTION: {
        // note: do it this way *just in case* there are non-integer display scales out there.
        e->motion.x = (int) ((float) e->motion.x * current_display_scale);
        e->motion.y = (int) ((float) e->motion.y * current_display_scale);
    } break;

    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN: {
        e->button.x = (int) ((float) e->button.x * current_display_scale);
        e->button.y = (int) ((float) e->button.y * current_display_scale);
    } break;
    }
}


int main(int argc, char *argv[])
{
    srand((unsigned int) time(NULL));

    Lt *lt = create_lt();

    int fps = 60;

    for (int i = 1; i < argc;) {
        if (strcmp(argv[i], "--fps") == 0) {
            if (i + 1 < argc) {
                if (sscanf(argv[i + 1], "%d", &fps) == 0) {
                    log_fail("Cannot parse FPS: %s is not a number\n", argv[i + 1]);
                    print_usage(stderr);
                    RETURN_LT(lt, -1);
                }
                i += 2;
            } else {
                log_fail("Value of FPS is not provided\n");
                print_usage(stderr);
                RETURN_LT(lt, -1);
            }
        } else {
            log_fail("Unknown flag %s\n", argv[i]);
            print_usage(stderr);
            RETURN_LT(lt, -1);
        }
    }

    if (SDL_Init(SDL_INIT_EVERYTHING & ~SDL_INIT_HAPTIC) < 0) {
        log_fail("Could not initialize SDL: %s\n", SDL_GetError());
        RETURN_LT(lt, -1);
    }
    PUSH_LT(lt, 42, SDL_Quit);

    setlocale(LC_NUMERIC, "C");

    SDL_ShowCursor(SDL_DISABLE);

    SDL_Window *const window = PUSH_LT(
        lt,
        SDL_CreateWindow(
            "Nothing",
            100, 100,
            SCREEN_WIDTH, SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI),
        SDL_DestroyWindow);

    if (window == NULL) {
        log_fail("Could not create SDL window: %s\n", SDL_GetError());
        RETURN_LT(lt, -1);
    }

    SDL_Renderer *const renderer = PUSH_LT(
        lt,
        SDL_CreateRenderer(window, -1, RENDERER_CONFIG),
        SDL_DestroyRenderer);
    if (renderer == NULL) {
        log_fail("Could not create SDL renderer: %s\n", SDL_GetError());
        RETURN_LT(lt, -1);
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    log_info("Using SDL Renderer: %s\n", info.name);

    if (SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) < 0) {
        log_fail("Could not set up blending mode for the renderer: %s\n", SDL_GetError());
        RETURN_LT(lt, -1);
    }

    SDL_Joystick *the_stick_of_joy = NULL;

    if (SDL_NumJoysticks() > 0) {
        the_stick_of_joy = PUSH_LT(lt, SDL_JoystickOpen(0), SDL_JoystickClose);

        if (the_stick_of_joy == NULL) {
            log_fail("Could not open 0th Stick of the Joy: %s\n", SDL_GetError());
            RETURN_LT(lt, -1);
        }

        log_info("Opened Joystick 0\n");
        log_info("Name: %s\n", SDL_JoystickNameForIndex(0));
        log_info("Number of Axes: %d\n", SDL_JoystickNumAxes(the_stick_of_joy));
        log_info("Number of Buttons: %d\n", SDL_JoystickNumButtons(the_stick_of_joy));
        log_info("Number of Balls: %d\n", SDL_JoystickNumBalls(the_stick_of_joy));

        SDL_JoystickEventState(SDL_ENABLE);
    } else {
        log_warn("Could not find any Sticks of the Joy\n");
    }

    // ------------------------------

    const char * sound_sample_files[] = {
        "./assets/sounds/nothing.wav",
        "./assets/sounds/something.wav",
        "./assets/sounds/dev/ding.wav",
        "./assets/sounds/dev/click.wav",
        "./assets/sounds/dev/save.wav"
    };
    const size_t sound_sample_files_count = sizeof(sound_sample_files) / sizeof(char*);

    Game *const game = PUSH_LT(
        lt,
        create_game(
            "./assets/levels/",
            sound_sample_files,
            sound_sample_files_count,
            renderer),
        destroy_game);
    if (game == NULL) {
        RETURN_LT(lt, -1);
    }

    // calculate the display scale for the first time.
    recalculate_display_scale(window, renderer);

    const Uint8 *const keyboard_state = SDL_GetKeyboardState(NULL);

    SDL_StopTextInput();
    SDL_Event e;
    const int64_t delta_time = (int64_t) roundf(1000.0f / 60.0f);
    int64_t render_timer = (int64_t) roundf(1000.0f / (float) fps);
    while (!game_over_check(game)) {
        const int64_t begin_frame_time = (int64_t) SDL_GetTicks();

        while (!game_over_check(game) && SDL_PollEvent(&e)) {

            // this function potentially fixes mouse events by scaling them according
            // to the window DPI scale. (eg. *2 on retina displays). it also updates
            // the cached DPI scale on window scale/move events.
            maybe_fixup_input_for_display_scale(window, renderer, &e);

            if (game_event(game, &e) < 0) {
                RETURN_LT(lt, -1);
            }
        }

        if (game_input(game, keyboard_state, the_stick_of_joy) < 0) {
            RETURN_LT(lt, -1);
        }

        if (game_update(game, (float) delta_time * 0.001f) < 0) {
            RETURN_LT(lt, -1);
        }

        if (game_sound(game) < 0) {
            RETURN_LT(lt, -1);
        }

        render_timer -= delta_time;
        if (render_timer <= 0) {
            if (game_render(game) < 0) {
                RETURN_LT(lt, -1);
            }
            SDL_RenderPresent(renderer);
            render_timer = (int64_t) roundf(1000.0f / (float) fps);
        }

        const int64_t end_frame_time = (int64_t) SDL_GetTicks();
        SDL_Delay((unsigned int) MAX(int64_t, 10, delta_time - (end_frame_time - begin_frame_time)));
    }

    RETURN_LT(lt, 0);
}
