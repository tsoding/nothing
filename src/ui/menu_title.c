#include <SDL.h>

#include "math/point.h"

#include "./menu_title.h"
#include "system/lt.h"
#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "system/str.h"
#include "game/camera.h"

struct MenuTitle
{
    Lt *lt;
    const char *text;
    Vec font_scale;
    float angle;
    Vec position;
};

MenuTitle *create_menu_title(const char *text,
                             Vec font_scale)
{
    trace_assert(text);

    Lt *lt = create_lt();

    MenuTitle *menu_title = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(MenuTitle)),
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

    menu_title->font_scale = font_scale;
    menu_title->angle = 0.0f;
    menu_title->position = vec(0.0f, 0.0f);

    return menu_title;
}

void destroy_menu_title(MenuTitle *menu_title)
{
    trace_assert(menu_title);
    RETURN_LT0(menu_title->lt);
}

int menu_title_render(const MenuTitle *menu_title,
                      Camera *camera)
{
    trace_assert(menu_title);
    trace_assert(camera);

    const size_t n = strlen(menu_title->text);
    char buf[2] = {0, 0};

    for (size_t i = 0; i < n; ++i) {
        buf[0] = menu_title->text[i];

        if (camera_render_text_screen(
                camera,
                buf,
                menu_title->font_scale,
                rgba(1.0f, 1.0f, 1.0f, 1.0f),
                vec_sum(
                    menu_title->position,
                    vec(
                        (float) (i * FONT_CHAR_WIDTH) * menu_title->font_scale.x,
                        sinf(menu_title->angle + (float) i / (float) n * 10.0f) * 20.0f))) < 0) {
            return -1;
        }
    }

    return 0;
}

int menu_title_update(MenuTitle *menu_title, float delta_time)
{
    trace_assert(menu_title);
    menu_title->angle = fmodf(menu_title->angle + 10.0f * delta_time, 2 * PI);
    return 0;
}

Vec menu_title_size(const MenuTitle *menu_title, const Camera *camera)
{
    trace_assert(menu_title);

    const Rect boundary = camera_text_boundary_box(
        camera,
        vec(0.0f, 0.0f),
        menu_title->font_scale,
        menu_title->text);

    return vec(boundary.w, boundary.h);
}

void menu_title_move(MenuTitle *menu_title,
                     Vec position)
{
    menu_title->position = position;
}
