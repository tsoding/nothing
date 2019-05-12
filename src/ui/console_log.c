#include "system/stacktrace.h"
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "color.h"
#include "game/sprite_font.h"
#include "console_log.h"
#include "math/point.h"
#include "system/str.h"
#include "system/lt.h"
#include "system/nth_alloc.h"

struct Console_Log
{
    Lt lt;

    const Sprite_font *font;
    Vec font_size;

    Color *colors;
    char **buffer;
    size_t cursor;
    size_t capacity;
};

Console_Log *create_console_log(const Sprite_font *font,
                Vec font_size,
                size_t capacity)
{
    Lt lt = {0};

    Console_Log *console_log = PUSH_LT(lt, nth_calloc(1, sizeof(Console_Log)), free);
    if (console_log == NULL) {
        RETURN_LT(lt, NULL);
    }
    console_log->lt = lt;
    console_log->font = font;
    console_log->font_size = font_size;
    console_log->capacity = capacity;

    console_log->buffer = PUSH_LT(lt, nth_calloc(capacity, sizeof(char*)), free);
    if (console_log->buffer == NULL) {
        RETURN_LT(lt, NULL);
    }

    console_log->colors = PUSH_LT(lt, nth_calloc(capacity, sizeof(Color)), free);
    if (console_log->colors == NULL) {
        RETURN_LT(lt, NULL);
    }

    console_log->cursor = 0;

    return console_log;
}

void destroy_console_log(Console_Log *console_log)
{
    trace_assert(console_log);
    for (size_t i = 0; i < console_log->capacity; ++i) {
        if (console_log->buffer[i]) {
            free(console_log->buffer[i]);
        }
    }
    RETURN_LT0(console_log->lt);
}

int console_log_render(const Console_Log *console_log,
               SDL_Renderer *renderer,
               Point position)
{
    trace_assert(console_log);
    trace_assert(renderer);
    (void) position;

    for (size_t i = 0; i < console_log->capacity; ++i) {
        const size_t j = (i + console_log->cursor) % console_log->capacity;
        if (console_log->buffer[j]) {
            if (sprite_font_render_text(console_log->font,
                                        renderer,
                                        vec_sum(position,
                                                vec(0.0f, FONT_CHAR_HEIGHT * console_log->font_size.y * (float) i)),
                                        console_log->font_size,
                                        console_log->colors[j],
                                        console_log->buffer[j]) < 0) {
                return -1;
            }
        }
    }

    return 0;
}

int console_log_push_line(Console_Log *console_log, const char *line, Color color)
{
    trace_assert(console_log);
    trace_assert(line);

    const size_t next_cursor = (console_log->cursor + 1) % console_log->capacity;

    if (console_log->buffer[console_log->cursor] != NULL) {
        free(console_log->buffer[console_log->cursor]);
    }

    console_log->buffer[console_log->cursor] = string_duplicate(line, NULL);
    console_log->colors[console_log->cursor] = color;

    if (console_log->buffer[console_log->cursor] == NULL) {
        return -1;
    }

    console_log->cursor = next_cursor;

    return 0;
}
