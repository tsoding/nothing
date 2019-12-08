#include <ctype.h>
#include <math.h>

#include "system/stacktrace.h"

#include "game.h"
#include "game/level.h"
#include "sdl/renderer.h"
#include "system/log.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "ui/console.h"
#include "ui/console_log.h"
#include "ui/edit_field.h"
#include "ui/history.h"
#include "math/extrema.h"

#define FONT_WIDTH_SCALE 3.0f
#define FONT_HEIGHT_SCALE 3.0f

#define CONSOLE_LOG_CAPACITY 10
#define HISTORY_CAPACITY 20
#define PROMPT_HEIGHT (FONT_HEIGHT_SCALE * FONT_CHAR_HEIGHT)
#define CONSOLE_LOG_HEIGHT (FONT_HEIGHT_SCALE * FONT_CHAR_HEIGHT * CONSOLE_LOG_CAPACITY)

#define CONSOLE_HEIGHT (CONSOLE_LOG_HEIGHT + PROMPT_HEIGHT)

#define SLIDE_DOWN_TIME 0.4f

#define CONSOLE_ALPHA (0.80f)
#define CONSOLE_BACKGROUND (rgba(0.20f, 0.20f, 0.20f, CONSOLE_ALPHA))
#define CONSOLE_FOREGROUND (rgba(0.80f, 0.80f, 0.80f, CONSOLE_ALPHA))
#define CONSOLE_ERROR (rgba(0.80f, 0.50f, 0.50f, CONSOLE_ALPHA))

typedef struct {
    const char *begin;
    const char *end;
} Token;


static inline
Token token(const char *begin, const char *end)
{
    Token t = {begin, end};
    return t;
}

static inline
int token_equals_str(Token t, const char *s)
{
    trace_assert(t.begin <= t.end);
    size_t n1 = (size_t) (t.end - t.begin);
    size_t n2 = strlen(s);
    if (n1 != n2) return 0;
    return memcmp(t.begin, s, n1) == 0;
}

static inline
Token token_nt(const char *s)
{
    return token(s, s + strlen(s));
}

static inline
void ltrim(Token *t)
{
    while (t->begin < t->end && isspace(*t->begin)) {
        t->begin++;
    }
}

static inline
Token chop_word(Token *t)
{
    ltrim(t);
    const char *end = t->begin;
    while (end < t->end && !isspace(*end)) {
        end++;
    }
    Token result = token(t->begin, end);
    t->begin = end;
    return result;
}

struct Console
{
    Lt *lt;
    Edit_field *edit_field;
    Console_Log *console_log;
    History *history;
    Game *game;
    float a;
};

/* TODO(#356): Console does not support autocompletion */
/* TODO(#358): Console does not support copy, cut, paste operations */

Console *create_console(Game *game)
{
    Lt *lt = create_lt();

    Console *console = PUSH_LT(lt, nth_calloc(1, sizeof(Console)), free);
    if (console == NULL) {
        RETURN_LT(lt, NULL);
    }
    console->lt = lt;

    console->edit_field = PUSH_LT(
        lt,
        create_edit_field(
            vec(FONT_WIDTH_SCALE, FONT_HEIGHT_SCALE),
            CONSOLE_FOREGROUND),
        destroy_edit_field);
    if (console->edit_field == NULL) {
        RETURN_LT(lt, NULL);
    }

    console->console_log = PUSH_LT(
        lt,
        create_console_log(
            vec(FONT_WIDTH_SCALE, FONT_HEIGHT_SCALE),
            CONSOLE_LOG_CAPACITY),
        destroy_console_log);

    console->a = 0;

    console->history = PUSH_LT(
        lt,
        create_history(HISTORY_CAPACITY),
        destroy_history);
    if (console->history == NULL) {
        RETURN_LT(lt, NULL);
    }

    console->game = game;

    return console;
}

void destroy_console(Console *console)
{
    trace_assert(console);
    RETURN_LT0(console->lt);
}

static int console_eval_input(Console *console)
{
    const char *source_code = edit_field_as_text(console->edit_field);

    /* TODO(#387): console pushes empty strings to the history */
    if (history_push(console->history, source_code) < 0) {
        return -1;
    }

    if (console_log_push_line(console->console_log, source_code, NULL, CONSOLE_FOREGROUND) < 0) {
        return -1;
    }

    Token input = token_nt(source_code);

    Token command = chop_word(&input);
    if (token_equals_str(command, "load")) {
        Token level = chop_word(&input);
        console_log_push_line(console->console_log, "Loading level:", NULL, CONSOLE_FOREGROUND);
        console_log_push_line(console->console_log, level.begin, level.end, CONSOLE_FOREGROUND);
        char level_name[256];
        memset(level_name, 0, 256);
        memcpy(level_name, level.begin, min_size_t((size_t)(level.end - level.begin), 255));

        if (game_load_level(console->game, level_name) < 0) {
            console_log_push_line(console->console_log, "Could not load level", NULL, CONSOLE_ERROR);
        }
    } else {
        console_log_push_line(console->console_log, "Unknown command", NULL, CONSOLE_ERROR);
    }

    edit_field_clean(console->edit_field);

    return 0;
}

int console_handle_event(Console *console,
                         const SDL_Event *event)
{
    switch(event->type) {
    case SDL_KEYDOWN: {
        switch(event->key.keysym.sym) {
        case SDLK_RETURN:
            return console_eval_input(console);

        case SDLK_UP:
            edit_field_replace(
                console->edit_field,
                history_current(console->history));
            history_prev(console->history);
            return 0;

        case SDLK_p: {
            if (event->key.keysym.mod & KMOD_CTRL) {
                edit_field_replace(
                    console->edit_field, history_current(console->history));
                history_prev(console->history);
                return 0;
            }
        } break;

        case SDLK_DOWN:
            edit_field_replace(
                console->edit_field,
                history_current(console->history));
            history_next(console->history);
            return 0;

        case SDLK_n: {
            if (event->key.keysym.mod & KMOD_CTRL) {
                edit_field_replace(
                    console->edit_field, history_current(console->history));
                history_next(console->history);
                return 0;
            }
        } break;
        }
    } break;
    }

    return edit_field_event(console->edit_field, event);
}

int console_render(const Console *console,
                   const Camera *camera)
{
    /* TODO(#364): console doesn't have any padding around the edit fields */
    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    const float e = console->a * (2 - console->a);
    const float y = -(1.0f - e) * CONSOLE_HEIGHT;

    if (camera_fill_rect_screen(
            camera,
            rect(0.0f, y,
                 (float) view_port.w,
                 CONSOLE_HEIGHT),
            CONSOLE_BACKGROUND) < 0) {
        return -1;
    }

    console_log_render(console->console_log,
                       camera,
                       vec(0.0f, y));

    if (edit_field_render_screen(console->edit_field,
                                 camera,
                                 vec(0.0f, y + CONSOLE_LOG_HEIGHT)) < 0) {
        return -1;
    }

    return 0;
}

int console_update(Console *console, float delta_time)
{
    trace_assert(console);

    if (console->a < 1.0f) {
        console->a += 1.0f / SLIDE_DOWN_TIME * delta_time;

        if (console->a > 1.0f) {
            console->a = 1.0f;
        }
    }

    return 0;
}

void console_slide_down(Console *console)
{
    trace_assert(console);
    console->a = 0.0f;
}
