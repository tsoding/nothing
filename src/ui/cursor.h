#ifndef CURSOR_H_
#define CURSOR_H_

#include <SDL.h>

#define CURSOR_ICON_WIDTH 32
#define CURSOR_ICON_HEIGHT 32

typedef enum {
    CURSOR_STYLE_POINTER = 0,
    CURSOR_STYLE_RESIZE_VERT,
    CURSOR_STYLE_RESIZE_HORIS,
    CURSOR_STYLE_RESIZE_DIAG1,
    CURSOR_STYLE_RESIZE_DIAG2,

    CURSOR_STYLE_N
} Cursor_Style;

static const char * const cursor_style_tex_files[CURSOR_STYLE_N] = {
    "./assets/images/cursor.bmp",
    "./assets/images/cursor-resize-vert.bmp",
    "./assets/images/cursor-resize-horis.bmp",
    "./assets/images/cursor-resize-diag1.bmp",
    "./assets/images/cursor-resize-diag2.bmp"
};

static const int cursor_style_tex_pivots[CURSOR_STYLE_N][2] = {
    {0, 0},
    {CURSOR_ICON_WIDTH / 2, CURSOR_ICON_HEIGHT / 2},
    {CURSOR_ICON_WIDTH / 2, CURSOR_ICON_HEIGHT / 2},
    {CURSOR_ICON_WIDTH / 2, CURSOR_ICON_HEIGHT / 2},
    {CURSOR_ICON_WIDTH / 2, CURSOR_ICON_HEIGHT / 2}
};

typedef struct {
    SDL_Texture *texs[CURSOR_STYLE_N];
    Cursor_Style style;
} Cursor;

int cursor_render(const Cursor *cursor, SDL_Renderer *renderer);

#endif  // CURSOR_H_
