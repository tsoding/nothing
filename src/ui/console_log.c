#include "system/stacktrace.h"
#include <stdlib.h>
#include <SDL.h>

#include "color.h"
#include "game/sprite_font.h"
#include "console_log.h"
#include "math/vec.h"
#include "system/str.h"
#include "system/lt.h"
#include "system/nth_alloc.h"

struct Console_Log
{
    Lt *lt;

    Vec2f font_size;

    Color *colors;
    char **buffer;
    size_t cursor;
    size_t capacity;
};

Console_Log *create_console_log(Vec2f font_size,
                                size_t capacity)
{
    Lt *lt = create_lt();

    Console_Log *console_log = PUSH_LT(lt, nth_calloc(1, sizeof(Console_Log)), free);
    if (console_log == NULL) {
        RETURN_LT(lt, NULL);
    }
    console_log->lt = lt;
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

void console_log_render(const Console_Log *console_log,
                        const Camera *camera,
                        Vec2f position)
{
    trace_assert(console_log);
    trace_assert(camera);

    for (size_t i = 0; i < console_log->capacity; ++i) {
        const size_t j = (i + console_log->cursor) % console_log->capacity;
        if (console_log->buffer[j]) {
            camera_render_text_screen(
                camera,
                console_log->buffer[j],
                console_log->font_size,
                console_log->colors[j],
                vec_sum(position,
                        vec(0.0f, FONT_CHAR_HEIGHT * console_log->font_size.y * (float) i)));
        }
    }
}

int console_log_push_line(Console_Log *console_log,
                          const char *line,
                          const char *line_end,
                          Color color)
{
    trace_assert(console_log);
    trace_assert(line);

    const size_t next_cursor = (console_log->cursor + 1) % console_log->capacity;

    if (console_log->buffer[console_log->cursor] != NULL) {
        free(console_log->buffer[console_log->cursor]);
    }

    console_log->buffer[console_log->cursor] = string_duplicate(line, line_end);
    console_log->colors[console_log->cursor] = color;

    if (console_log->buffer[console_log->cursor] == NULL) {
        return -1;
    }

    console_log->cursor = next_cursor;

    return 0;
}

void console_log_clear(Console_Log *console_log)
{
    trace_assert(console_log);
    console_log->cursor = 0;
    for (size_t i = 0; i < console_log->capacity; ++i) {
        if (console_log->buffer[i]) {
            free(console_log->buffer[i]);
            console_log->buffer[i] = 0;
        }
    }
}
