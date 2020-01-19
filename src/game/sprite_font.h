#ifndef SPRITE_FONT_H_
#define SPRITE_FONT_H_

#include "color.h"
#include "math/vec.h"
#include "math/rect.h"

#define FONT_CHAR_WIDTH 7
#define FONT_CHAR_HEIGHT 9

typedef struct {
    SDL_Texture *texture;
} Sprite_font;

SDL_Texture *load_bmp_font_texture(SDL_Renderer *renderer,
                                   const char *bmp_file_path);

void sprite_font_render_text(const Sprite_font *sprite_font,
                             SDL_Renderer *renderer,
                             Vec2f position,
                             Vec2f size,
                             Color color,
                             const char *text);

static inline
Rect sprite_font_boundary_box(Vec2f position, Vec2f size, const char *text)
{
    size_t num_max_col = 1, num_row = 1;
    for (size_t i = 0, num_col = 1; text[i] != '\0'; i++){
        if (text[i] == '\n'){
            num_col = 1;
            num_row++;
            continue;
        }
        if (num_col > num_max_col)
            num_max_col = num_col;
        num_col++;
    }
    return rect(
        position.x, position.y,
        size.x * FONT_CHAR_WIDTH * (float) num_max_col,
        size.y * FONT_CHAR_HEIGHT * (float) num_row);
}

#endif  // SPRITE_FONT_H_
