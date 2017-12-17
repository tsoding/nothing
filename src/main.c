#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "./player.h"
#include "./platforms.h"
#include "./error.h"
#include "./game.h"
#include "./lt.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GAME_FPS 60
#define LEVEL_FILE_NAME "./levels/platforms.txt"

/* LT module adapter for SDL_Quit */
static void SDL_Quit_lt(void* ignored)
{
    (void) ignored;
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    (void) argc;                /* unused */
    (void) argv;                /* unused */

    lt_t *const lt = create_lt();

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        print_error_msg(ERROR_TYPE_SDL2, "Could not initialize SDL");
        LT_RETURN(lt, -1);
    }
    LT_PUSH(lt, 42, SDL_Quit_lt);

    SDL_Window *const window = LT_PUSH(
        lt,
        SDL_CreateWindow("Nothing", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN),
        SDL_DestroyWindow);

    if (window == NULL) {
        print_error_msg(ERROR_TYPE_SDL2, "Could not create SDL window");
        LT_RETURN(lt, -1);
    }

    SDL_Renderer *const renderer = LT_PUSH(
        lt,
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
        SDL_DestroyRenderer);
    if (renderer == NULL) {
        print_error_msg(ERROR_TYPE_SDL2, "Could not create SDL renderer");
        LT_RETURN(lt, -1);
    }

    SDL_Joystick *the_stick_of_joy = NULL;

    if (SDL_NumJoysticks() > 0) {
        the_stick_of_joy = LT_PUSH(lt, SDL_JoystickOpen(0), SDL_JoystickClose);

        if (the_stick_of_joy == NULL) {
            print_error_msg(ERROR_TYPE_SDL2, "Could not open 0th Stick of the Joy: %s\n");
            LT_RETURN(lt, -1);
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

    game_t *const game = LT_PUSH(lt, create_game(LEVEL_FILE_NAME), destroy_game);
    if (game == NULL) {
        print_current_error_msg("Could not create the game object");
        LT_RETURN(lt, -1);
    }

    const Uint8 *const keyboard_state = SDL_GetKeyboardState(NULL);
    const Uint32 delay_ms = (Uint32) roundf(1000.0f / GAME_FPS);
    SDL_Event e;
    while (!is_game_over(game)) {
        while (!is_game_over(game) && SDL_PollEvent(&e)) {
            if (game_event(game, &e) < 0) {
                print_current_error_msg("Failed handling event");
                LT_RETURN(lt, -1);
            }
        }

        if (game_input(game, keyboard_state, the_stick_of_joy) < 0) {
            print_current_error_msg("Failed handling input");
            LT_RETURN(lt, -1);
        }

        if (game_update(game, delay_ms) < 0) {
            print_current_error_msg("Failed handling updating");
            LT_RETURN(lt, -1);
        }

        if (game_render(game, renderer) < 0) {
            print_current_error_msg("Failed rendering the game");
            LT_RETURN(lt, -1);
        }
    }

    LT_RETURN(lt, 0);
}
