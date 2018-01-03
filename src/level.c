#include <assert.h>
#include <SDL2/SDL.h>

#include "./level.h"

level_t *create_level(player_t *player,
                      platforms_t *platforms)
{
    assert(player);
    assert(platforms);
    return NULL;
}

void destroy_level(level_t *level)
{
    assert(level);
}

int level_render(const level_t *level, SDL_Renderer *renderer)
{
    assert(level);
    assert(renderer);
    return 0;
}

int level_update(level_t *level, Uint32 delta_time)
{
    assert(level);
    assert(delta_time > 0);

    return 0;
}

int level_event(level_t *level, const SDL_Event *event)
{
    assert(level);
    assert(event);

    return 0;
}

int level_input(level_t *level,
                const Uint8 *const keyboard_state,
                SDL_Joystick *the_stick_of_joy)
{
    assert(level);
    assert(keyboard_state);
    (void) the_stick_of_joy;

    return 0;
}

int is_level_over(const level_t *level)
{
    assert(level);

    return 0;
}
