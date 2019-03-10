#include <SDL2/SDL.h>

#include "math/point.h"

#include "./menu_title.h"
#include "system/lt.h"
#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "system/str.h"
#include "game/sprite_font.h"

struct MenuTitle
{
    Lt *lt;
    const char *text;
    const Sprite_font *sprite_font;
};

MenuTitle *create_menu_title(const char *text,
                             const Sprite_font *sprite_font)
{
    trace_assert(text);

    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    MenuTitle *menu_title = PUSH_LT(
        lt,
        nth_alloc(sizeof(MenuTitle)),
        free);
    if (menu_title == NULL) {
        RETURN_LT(lt, NULL);
    }
    menu_title->lt = lt;

    menu_title->text = PUSH_LT(
        lt,
        string_duplicate(text, NULL),
        free);
    if (menu_title->text == NULL) {
        RETURN_LT(lt, NULL);
    }

    menu_title->sprite_font = sprite_font;

    return menu_title;
}

void destroy_menu_title(MenuTitle *menu_title)
{
    trace_assert(menu_title);
    RETURN_LT0(menu_title->lt);
}

int menu_title_render(const MenuTitle *menu_title,
                      SDL_Renderer *renderer,
                      Vec position)
{
    trace_assert(menu_title);
    trace_assert(renderer);

    if (sprite_font_render_text(
            menu_title->sprite_font,
            renderer,
            position,
            vec(5.0f, 5.0f),
            rgba(1.0f, 1.0f, 1.0f, 1.0f),
            menu_title->text) < 0) {
        return -1;
    }

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

    Rect boundary = sprite_font_boundary_box(
        menu_title->sprite_font,
        vec(0.0f, 0.0f),
        vec(5.0f, 5.0f),
        menu_title->text);

    return vec(boundary.w, boundary.h);
}
