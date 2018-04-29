#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "./game/level/player.h"
#include "./platforms.h"
#include "./error.h"
#include "./game.h"
#include "./lt.h"
#include "./path.h"
#include "./point.h"
#include "./game/sound_medium.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GAME_FPS 60

/* LT module adapter for Mix_CloseAudio */
static void Mix_CloseAudio_lt(void* ignored)
{
    (void) ignored;
    Mix_CloseAudio();
}

/* LT module adapter for SDL_Quit */
static void SDL_Quit_lt(void* ignored)
{
    (void) ignored;
    SDL_Quit();
}

static void print_usage(FILE *stream)
{
    fprintf(stream, "Usage: nothing <level-file>\n");
}

int main(int argc, char *argv[])
{
    srand((unsigned int) time(NULL));

    lt_t *const lt = create_lt();

    if (argc < 2) {
        print_usage(stderr);
        RETURN_LT(lt, -1);
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        print_error_msg(ERROR_TYPE_SDL2, "Could not initialize SDL");
        RETURN_LT(lt, -1);
    }
    PUSH_LT(lt, 42, SDL_Quit_lt);

    SDL_Window *const window = PUSH_LT(
        lt,
        SDL_CreateWindow(
            "Nothing",
            100, 100,
            SCREEN_WIDTH, SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE),
        SDL_DestroyWindow);

    if (window == NULL) {
        print_error_msg(ERROR_TYPE_SDL2, "Could not create SDL window");
        RETURN_LT(lt, -1);
    }

    SDL_Renderer *const renderer = PUSH_LT(
        lt,
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
        SDL_DestroyRenderer);
    if (renderer == NULL) {
        print_error_msg(ERROR_TYPE_SDL2, "Could not create SDL renderer");
        RETURN_LT(lt, -1);
    }
    if (SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) < 0) {
        print_error_msg(ERROR_TYPE_SDL2, "Could not set up blending mode for the renderer");
        RETURN_LT(lt, -1);
    }

    SDL_Joystick *the_stick_of_joy = NULL;

    if (SDL_NumJoysticks() > 0) {
        the_stick_of_joy = PUSH_LT(lt, SDL_JoystickOpen(0), SDL_JoystickClose);

        if (the_stick_of_joy == NULL) {
            print_error_msg(ERROR_TYPE_SDL2, "Could not open 0th Stick of the Joy: %s\n");
            RETURN_LT(lt, -1);
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

    if (Mix_OpenAudio(
            MIX_DEFAULT_FREQUENCY,
            MIX_DEFAULT_FORMAT,
            2,
            1024) < 0) {
        print_error_msg(ERROR_TYPE_SDL2_MIXER, "Could not initialize the audio\n");
        RETURN_LT(lt, -1);
    }
    PUSH_LT(lt, 42, Mix_CloseAudio_lt);

    Mix_Chunk * sound_samples[] = {
        PUSH_LT(lt, Mix_LoadWAV("./sounds/nothing.wav"), Mix_FreeChunk),
        PUSH_LT(lt, Mix_LoadWAV("./sounds/something.wav"), Mix_FreeChunk)
    };
    const size_t sound_samples_count = sizeof(sound_samples) / sizeof(Mix_Chunk*);

    sound_medium_t *sound_medium =
        PUSH_LT(
            lt,
            create_sound_medium(sound_samples, sound_samples_count, 0, MIX_CHANNELS - 1),
            destroy_sound_medium);
    if (sound_medium == NULL) {
        print_current_error_msg("Could not create sound medium");
        RETURN_LT(lt, -1);
    }

    // ------------------------------

    char *sounds_folder = PUSH_LT(lt, base_path_folder("sounds"), free);
    if (sounds_folder == NULL) {
        print_current_error_msg("Could not get the sounds folder");
        RETURN_LT(lt, -1);
    }

    game_t *const game = PUSH_LT(lt, create_game(argv[1], sound_medium), destroy_game);
    if (game == NULL) {
        print_current_error_msg("Could not create the game object");
        RETURN_LT(lt, -1);
    }

    const Uint8 *const keyboard_state = SDL_GetKeyboardState(NULL);
    const float delay_ms = 1.0f / GAME_FPS;
    SDL_Event e;
    while (!game_over_check(game)) {
        while (!game_over_check(game) && SDL_PollEvent(&e)) {
            if (game_event(game, &e) < 0) {
                print_current_error_msg("Failed handling event");
                RETURN_LT(lt, -1);
            }
        }

        if (game_input(game, keyboard_state, the_stick_of_joy) < 0) {
            print_current_error_msg("Failed handling input");
            RETURN_LT(lt, -1);
        }

        if (game_render(game, renderer) < 0) {
            print_current_error_msg("Failed rendering the game");
            RETURN_LT(lt, -1);
        }

        if (game_update(game, delay_ms) < 0) {
            print_current_error_msg("Failed handling updating");
            RETURN_LT(lt, -1);
        }

        if (game_sound(game) < 0) {
            print_current_error_msg("Failed handling the sound");
            RETURN_LT(lt, -1);
        }
    }

    RETURN_LT(lt, 0);
}
