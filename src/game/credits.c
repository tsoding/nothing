#include <stdio.h>

#include "./credits.h"
#include "game/level/background.h"
#include "game/sprite_font.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/stacktrace.h"
#include "system/str.h"
#include "ui/list_selector.h"
#include "system/log.h"
#include "game/contributors.h"
#include "ui/wiggly_text.h"

#define TITLE_MARGIN_TOP 100.0f
#define TITLE_MARGIN_BOTTOM 100.0f

struct Credits
{
    Lt *lt;
    Background background;
    Vec2f camera_position;
    Contributors *contributors;
    WigglyText wiggly_text;
    ListSelector *list_selector;
};

Credits *create_credits(const Sprite_font *sprite_font)
{
    Lt *lt = create_lt();

    Credits *credits = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(Credits)),
        free);
    if (credits == NULL) {
        RETURN_LT(lt, NULL);
    }
    credits->lt = lt;

    credits->background = create_background(hexstr("073642"));

    credits->camera_position = vec(0.0f, 0.0f);

    credits->contributors = PUSH_LT(
        lt,
        create_contributors(),
        destroy_contributors);
    if (credits->contributors == NULL) {
        RETURN_LT(lt, NULL);
    }

    credits->wiggly_text = (WigglyText) {
        .text = "Contributors",
        .scale = {10.0f, 10.0f},
        .color = COLOR_WHITE,
    };

    credits->list_selector = PUSH_LT(
        lt,
        create_list_selector(
            sprite_font,
            contributors_names(credits->contributors),
            contributors_count(credits->contributors),
            vec(5.0f, 5.0f),
            50.0f),
        destroy_list_selector);
    if (credits->list_selector == NULL) {
        RETURN_LT(lt, NULL);
    }

    return credits;
}

void destroy_credits(Credits *credits)
{
    trace_assert(credits);
    RETURN_LT0(credits->lt);
}

int credits_render(const Credits *credits,
                        const Camera *camera)
{
    trace_assert(credits);

    const Rect viewport = camera_view_port_screen(camera);

    if (background_render(&credits->background, camera) < 0) {
        return -1;
    }

    const Vec2f title_size = wiggly_text_size(&credits->wiggly_text, camera);

    if (wiggly_text_render(
            &credits->wiggly_text,
            camera,
            vec(viewport.w * 0.5f - title_size.x * 0.5f, TITLE_MARGIN_TOP)) < 0) {
        return -1;
    }

    if (list_selector_render(credits->list_selector, camera->renderer) < 0) {
        return -1;
    }

    return 0;
}

int credits_update(Credits *credits,
                        float delta_time)
{
    trace_assert(credits);
    vec_add(&credits->camera_position,
            vec(0.0f, 15.0f * delta_time));

    if (wiggly_text_update(&credits->wiggly_text, delta_time) < 0) {
        return -1;
    }

    return 0;
}

int credits_event(Credits *credits,
                       const SDL_Event *event,
                       const Camera *camera)
{
    trace_assert(credits);
    trace_assert(event);
    // trace_assert(credits->list_selector); // why null-pointer?
    switch (event->type) {
    case SDL_WINDOWEVENT: {
        switch (event->window.event) {
        case SDL_WINDOWEVENT_SHOWN:
        case SDL_WINDOWEVENT_RESIZED: {
            const Vec2f font_scale = vec(5.0f, 5.0f);
            const float padding_bottom = 50.0f;

            int width;
            SDL_GetWindowSize(SDL_GetWindowFromID(event->window.windowID), &width, NULL);

            const Vec2f title_size = wiggly_text_size(&credits->wiggly_text, camera);

            const Vec2f selector_size = list_selector_size(
                credits->list_selector,
                font_scale,
                padding_bottom);

            list_selector_move(
                credits->list_selector,
                vec((float)width * 0.5f - selector_size.x * 0.5f,
                    TITLE_MARGIN_TOP + title_size.y + TITLE_MARGIN_BOTTOM));
        } break;
        }
    } break;
    }

    return 0;//list_selector_event(credits->list_selector, event);
}

int credits_input(Credits *credits,
                       const Uint8 *const keyboard_state,
                       SDL_Joystick *the_stick_of_joy)
{
    trace_assert(credits);
    trace_assert(keyboard_state);
    (void) the_stick_of_joy;
    return 0;
}

const char *credits_selected_contributor(const Credits *credits)
{
    trace_assert(credits);

    const int selected_index = list_selector_selected(credits->list_selector);
    if (selected_index < 0) {
        return NULL;
    }

    const char **names = contributors_names(credits->contributors);

    return names[selected_index];
}

void credits_clean_selection(Credits *credits)
{
    trace_assert(credits);
    list_selector_clean_selection(credits->list_selector);
}

int credits_enter_camera_event(Credits *credits,
                                    Camera *camera)
{
    camera_center_at(camera, credits->camera_position);
    return 0;
}
