#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "./player.h"
#include "./platforms.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GAME_FPS 60
#define GROUND_LEVEL 200.0f

int main(int argc, char *argv[])
{
    (void) argc;                /* unused */
    (void) argv;                /* unused */

    int exit_code = 0;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s", SDL_GetError());
        exit_code = -1;
        goto sdl_init_fail;
    }

    SDL_Window *const window = SDL_CreateWindow("Nothing",
                                                100, 100,
                                                SCREEN_WIDTH, SCREEN_HEIGHT,
                                                SDL_WINDOW_SHOWN);

    if (window == NULL) {
        fprintf(stderr, "Could not create SDL window: %s", SDL_GetError());
        exit_code = -1;
        goto sdl_create_window_fail;
    }

    SDL_Renderer *const renderer = SDL_CreateRenderer(window, -1,
                                                      SDL_RENDERER_ACCELERATED |
                                                      SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "Could not create SDL renderer: %s", SDL_GetError());
        exit_code = -1;
        goto sdl_create_renderer_fail;
    }

    SDL_Joystick *the_stick_of_joy = NULL;

    if (SDL_NumJoysticks() > 0) {
        the_stick_of_joy = SDL_JoystickOpen(0);

        if (the_stick_of_joy == NULL) {
            fprintf(stderr, "Could not open 0th Stick of the Joy\n");
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
        fprintf(stderr, "[WARNING] Could not find any The Sticks of the Joy\n");
    }

    // ------------------------------

    player_t *const player = create_player(100.0f, 0.0f);
    if (player == NULL) {
        perror("Could not create player");
        exit_code = -1;
        goto create_player_fail;
    }

    platforms_t *platforms = load_platforms_from_file("./platforms.txt");
    if (platforms == NULL) {
        perror("Could not read platforms from ./platforms.txt");
        exit_code = -1;
        goto create_platforms_fail;
    }

    camera_t *const camera = create_camera(vec(0.0f, 0.0f));
    if (camera == NULL) {
        perror("Could not create camera");
        exit_code = -1;
        goto create_camera_fail;
    }

    int quit = 0;

    const Uint8 *const keyboard_state = SDL_GetKeyboardState(NULL);
    const Uint32 delay_ms = (Uint32) roundf(1000.0f / GAME_FPS);
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                quit = 1;
                break;

            case SDL_KEYDOWN:
                switch (e.key.keysym.sym) {
                case SDLK_SPACE:
                    player_jump(player);
                    break;

                case SDLK_q:
                    printf("reloading...");

                    destroy_platforms(platforms);
                    platforms = load_platforms_from_file("./platforms.txt");

                    if (platforms == NULL) {
                        exit_code = -1;
                        goto reload_platforms_failed;
                    }
                    break;
                }
                break;

            case SDL_JOYBUTTONDOWN:
                printf("Button: %d\n", e.jbutton.button);
                if (e.jbutton.button == 1) {
                    player_jump(player);
                }
                break;
            }

        }

        if (keyboard_state[SDL_SCANCODE_A]) {
            player_move_left(player);
        } else if (keyboard_state[SDL_SCANCODE_D]) {
            player_move_right(player);
        } else if (the_stick_of_joy && SDL_JoystickGetAxis(the_stick_of_joy, 0) < 0) {
            player_move_left(player);
        } else if (the_stick_of_joy && SDL_JoystickGetAxis(the_stick_of_joy, 0) > 0) {
            player_move_right(player);
        } else {
            player_stop(player);
        }

        update_player(player, platforms, delay_ms);
        player_focus_camera(player, camera);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        render_player(player, renderer, camera);
        render_platforms(platforms, renderer, camera);
        SDL_RenderPresent(renderer);
        SDL_Delay(delay_ms);
    }

reload_platforms_failed:
    destroy_camera(camera);
create_camera_fail:
    if (platforms) { destroy_platforms(platforms); }
create_platforms_fail:
    destroy_player(player);
create_player_fail:
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
