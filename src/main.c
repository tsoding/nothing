#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "./player.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GAME_FPS 60

int main(int argc, char *argv[])
{
    int exit_code = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s", SDL_GetError());
        exit_code = -1;
        goto sdl_init_fail;
    }

    SDL_Window *window = SDL_CreateWindow("Nothing",
                                          100, 100,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "Could not create SDL window: %s", SDL_GetError());
        exit_code = -1;
        goto sdl_create_window_fail;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                                SDL_RENDERER_ACCELERATED |
                                                SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "Could not create SDL renderer: %s", SDL_GetError());
        exit_code = -1;
        goto sdl_create_renderer_fail;
    }

    struct player *player = create_player(0.0f, 200.0f);
    if (player == NULL) {
        perror("Could not create player");
        exit_code = -1;
        goto create_player_fail;
    }

    int quit = 0;

    const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
    const int delay_ms = roundf(1000.0 / GAME_FPS);
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                quit = 1;
                break;
            }

        }

        if (keyboard_state[SDL_SCANCODE_A]) {
            player_move_left(player);
        } else if (keyboard_state[SDL_SCANCODE_D]) {
            player_move_right(player);
        } else {
            player_stop(player);
        }

        update_player(player, delay_ms);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        render_player(player, renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(delay_ms);
    }

    destroy_player(player);
create_player_fail:
    SDL_DestroyRenderer(renderer);
sdl_create_renderer_fail:
    SDL_DestroyWindow(window);
sdl_create_window_fail:
    SDL_Quit();
sdl_init_fail:
    return exit_code;
}
