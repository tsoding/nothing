#include <SDL2/SDL.h>

#include "math/point.h"

#include "system/stacktrace.h"
#include "./menu_title.h"

MenuTitle *create_menu_title(const char *text)
{
    trace_assert(text);
    return NULL;
}

void destroy_menu_title(MenuTitle *menu_title)
{
    trace_assert(menu_title);
}

int menu_title_render(const MenuTitle *menu_title,
                      SDL_Renderer *renderer,
                      Vec position)
{
    trace_assert(menu_title);
    trace_assert(renderer);
    (void) position;
    return 0;
}

int menu_title_update(MenuTitle *menu_title, float delta_time)
{
    trace_assert(menu_title);
    (void) delta_time;
    return 0;
}

Vec menu_title_size(const MenuTitle *menu_title)
{
    trace_assert(menu_title);
    return vec(0.0f, 0.0f);
}
