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
#include "math/point.h"
#include "sdl/renderer.h"
#include "system/log.h"
#include "system/lt.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

static void print_usage(FILE *stream)
{
    fprintf(stream, "Usage: nothing [--fps <fps>]\n");
}

int main(int argc, char *argv[])
{
    srand((unsigned int) time(NULL));

    Lt *lt = create_lt();

    int fps = 30;

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

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
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
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE),
        SDL_DestroyWindow);

    if (window == NULL) {
        log_fail("Could not create SDL window: %s\n", SDL_GetError());
        RETURN_LT(lt, -1);
    }

    SDL_Renderer *const renderer = PUSH_LT(
        lt,
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
        SDL_DestroyRenderer);
    if (renderer == NULL) {
        log_fail("Could not create SDL renderer: %s\n", SDL_GetError());
        RETURN_LT(lt, -1);
    }
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
        "./sounds/nothing.wav",
        "./sounds/something.wav"
    };
    const size_t sound_sample_files_count = sizeof(sound_sample_files) / sizeof(char*);

    Game *const game = PUSH_LT(
        lt,
        create_game(
            "./levels/",
            sound_sample_files,
            sound_sample_files_count,
            renderer),
        destroy_game);
    if (game == NULL) {
        RETURN_LT(lt, -1);
    }

    const Uint8 *const keyboard_state = SDL_GetKeyboardState(NULL);

    SDL_StopTextInput();
    SDL_Event e;
    const int64_t delta_time = (int64_t) roundf(1000.0f / 60.0f);
    int64_t render_timer = (int64_t) roundf(1000.0f / (float) fps);
    while (!game_over_check(game)) {
        const int64_t begin_frame_time = (int64_t) SDL_GetTicks();

        while (!game_over_check(game) && SDL_PollEvent(&e)) {
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
        SDL_Delay((unsigned int) max_int64(10, delta_time - (end_frame_time - begin_frame_time)));
    }

    RETURN_LT(lt, 0);
}
