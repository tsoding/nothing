#ifndef SPRITE_FONT_H_
#define SPRITE_FONT_H_

#include "color.h"
#include "math/point.h"
#include "math/rect.h"

#define FONT_CHAR_WIDTH 7
#define FONT_CHAR_HEIGHT 9

typedef struct Sprite_font Sprite_font;

Sprite_font *create_sprite_font_from_file(const char *bmp_file_path,
                                            SDL_Renderer *renderer);
void destroy_sprite_font(Sprite_font *sprite_font);

int sprite_font_render_text(const Sprite_font *sprite_font,
                            SDL_Renderer *renderer,
                            Vec position,
                            Vec size,
                            Color color,
                            const char *text);

Rect sprite_font_boundary_box(const Sprite_font *sprite_font,
                              Vec position,
                              Vec size,
                              const char *text);

#endif  // SPRITE_FONT_H_
