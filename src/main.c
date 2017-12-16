#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "./player.h"
#include "./platforms.h"
#include "./error.h"
#include "./game.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GAME_FPS 60
#define LEVEL_FILE_NAME "./levels/platforms.txt"

int main(int argc, char *argv[])
{
    (void) argc;                /* unused */
    (void) argv;                /* unused */

    int exit_code = 0;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        print_error_msg(ERROR_TYPE_SDL2, "Could not initialize SDL");
        exit_code = -1;
        goto sdl_init_fail;
    }

    SDL_Window *const window = SDL_CreateWindow("Nothing",
                                                100, 100,
                                                SCREEN_WIDTH, SCREEN_HEIGHT,
                                                SDL_WINDOW_SHOWN);

    if (window == NULL) {
        print_error_msg(ERROR_TYPE_SDL2, "Could not create SDL window");
        exit_code = -1;
        goto sdl_create_window_fail;
    }

    SDL_Renderer *const renderer = SDL_CreateRenderer(window, -1,
                                                      SDL_RENDERER_ACCELERATED |
                                                      SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        print_error_msg(ERROR_TYPE_SDL2, "Could not create SDL renderer");
        exit_code = -1;
        goto sdl_create_renderer_fail;
    }

    SDL_Joystick *the_stick_of_joy = NULL;

    if (SDL_NumJoysticks() > 0) {
        the_stick_of_joy = SDL_JoystickOpen(0);

        if (the_stick_of_joy == NULL) {
            print_error_msg(ERROR_TYPE_SDL2, "Could not open 0th Stick of the Joy: %s\n");
            exit_code = -1;
            goto sdl_joystick_open_fail;
        }

        printf("Opened Joystick 0\n");
        printf("Name: %s\n", SDL_JoystickNameForIndex(0));
        printf("Number of Axes: %d\n", SDL_JoystickNumAxes(the_stick_of_joy));
        printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(the_stick_of_joy));
        printf("Number of Balls: %d\n", SDL_JoystickNumBalls(the_stick_of_joy));

        SDL_JoystickEventState(SDL_ENABLE);
    } else {
        fprintf(stderr, "[WARNING] Could not find any Sticks of the Joy\n");
    }

    // ------------------------------

    game_t *const game = create_game(LEVEL_FILE_NAME);
    if (game == NULL) {
        print_current_error_msg("Could not create the game object");
        exit_code = -1;
        goto create_game_failed;
    }

    const Uint8 *const keyboard_state = SDL_GetKeyboardState(NULL);
    const Uint32 delay_ms = (Uint32) roundf(1000.0f / GAME_FPS);
    SDL_Event e;
    while (!is_game_over(game)) {
        while (!is_game_over(game) && SDL_PollEvent(&e)) {
            if (game_event(game, &e) < 0) {
                print_current_error_msg("Failed handling event");
                exit_code = -1;
                goto game_failed;
            }
        }

        if (game_input(game, keyboard_state, the_stick_of_joy) < 0) {
            print_current_error_msg("Failed handling input");
            exit_code = -1;
            goto game_failed;
        }

        if (game_update(game, delay_ms) < 0) {
            print_current_error_msg("Failed handling updating");
            exit_code = -1;
            goto game_failed;
        }

        if (game_render(game, renderer) < 0) {
            print_current_error_msg("Failed rendering the game");
            exit_code = -1;
            goto game_failed;
        }
    }

game_failed:
    destroy_game(game);
create_game_failed:
    if (the_stick_of_joy) { SDL_JoystickClose(the_stick_of_joy); }
sdl_joystick_open_fail:
    SDL_DestroyRenderer(renderer);
sdl_create_renderer_fail:
    SDL_DestroyWindow(window);
sdl_create_window_fail:
    SDL_Quit();
sdl_init_fail:
    return exit_code;
}
