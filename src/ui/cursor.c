#include "system/stacktrace.h"
#include "cursor.h"

int cursor_render(const Cursor *cursor, SDL_Renderer *renderer)
{
    trace_assert(cursor);
    trace_assert(renderer);

    int cursor_x, cursor_y;
    SDL_GetMouseState(&cursor_x, &cursor_y);

    const SDL_Rect src = {0, 0, 32, 32};
    const SDL_Rect dest = {cursor_x, cursor_y, 32, 32};
    if (SDL_RenderCopy(
            renderer,
            cursor->texs[cursor->style],
            &src, &dest) < 0) {
        return -1;
    }

    return 0;
}
