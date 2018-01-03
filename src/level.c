#include <assert.h>
#include <SDL2/SDL.h>

#include "./lt.h"
#include "./player.h"
#include "./platforms.h"
#include "./level.h"
#include "./error.h"

struct level_t
{
    lt_t *lt;
    player_t *player;
    platforms_t *platforms;
};

level_t *create_level(player_t *player,
                      platforms_t *platforms)
{
    assert(player);
    assert(platforms);

    lt_t *const lt = create_lt();
    if (lt == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        return NULL;
    }

    level_t *const level = PUSH_LT(lt, malloc(sizeof(level_t)), free);
    if (level == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    level->player = PUSH_LT(lt, player, destroy_player);
    level->platforms = PUSH_LT(lt, platforms, destroy_platforms);
    level->lt = lt;

    return NULL;
}

void destroy_level(level_t *level)
{
    assert(level);
    RETURN_LT0(level->lt);
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
