#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "./player.h"
#include "./platforms.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GAME_FPS 30
#define GROUND_LEVEL 200.0f

int main(int argc, char *argv[])
{
    (void) argc;                /* unused */
    (void) argv;                /* unused */

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

    struct player_t *player = create_player(100.0f, 0.0f);
    if (player == NULL) {
        perror("Could not create player");
        exit_code = -1;
        goto create_player_fail;
    }

    const struct rect_t platforms_rects[] = {
        { .x = 0.0f,
          .y = GROUND_LEVEL + 50.0f,
          .w = 50.0f,
          .h = 50.0f },
        { .x = 150.0f,
          .y = GROUND_LEVEL + 50.0f,
          .w = SCREEN_WIDTH - 150.0f,
          .h = 50.0f },
        { .x = 0.0f,
          .y = GROUND_LEVEL + 100.0f,
          .w = 50.0f,
          .h = 50.0f },
        { .x = 150.0f,
          .y = GROUND_LEVEL + 100.0f,
          .w = 50.0f,
          .h = 50.0f },
        { .x = 0.0f,
          .y = GROUND_LEVEL + 150.0f,
          .w = 50.0f,
          .h = 50.0f },
        { .x = 150.0f,
          .y = GROUND_LEVEL + 150.0f,
          .w = 50.0f,
          .h = 50.0f }
    };
    struct platforms_t *platforms = create_platforms(
        platforms_rects,
        sizeof(platforms_rects) / sizeof(struct rect_t));
    if (platforms == NULL) {
        perror("Could not create platforms");
        exit_code = -1;
        goto create_platforms_fail;
    }

    int quit = 0;

    const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
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
                }
            }

        }

        if (keyboard_state[SDL_SCANCODE_A]) {
            player_move_left(player);
        } else if (keyboard_state[SDL_SCANCODE_D]) {
            player_move_right(player);
        } else {
            player_stop(player);
        }

        update_player(player, platforms, delay_ms);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        render_player(player, renderer);
        render_platforms(platforms, renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(delay_ms);
    }

    destroy_platforms(platforms);
create_platforms_fail:
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
