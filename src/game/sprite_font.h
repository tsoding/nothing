#ifndef SPRITE_FONT_H_
#define SPRITE_FONT_H_

#include "color.h"
#include "math/point.h"
#include "math/rect.h"

typedef struct sprite_font_t sprite_font_t;

sprite_font_t *create_sprite_font_from_file(const char *bmp_file_path,
                                            SDL_Renderer *renderer);
void destroy_sprite_font(sprite_font_t *sprite_font);

int sprite_font_render_text(const sprite_font_t *sprite_font,
                            SDL_Renderer *renderer,
                            vec_t position,
                            vec_t size,
                            color_t color,
                            const char *text);

rect_t sprite_font_boundary_box(const sprite_font_t *sprite_font,
                                vec_t position,
                                vec_t size,
                                const char *text);

#endif  // SPRITE_FONT_H_
