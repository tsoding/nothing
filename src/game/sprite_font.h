#ifndef SPRITE_FONT_H_
#define SPRITE_FONT_H_

#include "math/point.h"

typedef struct sprite_font_t sprite_font_t;

sprite_font_t *create_sprite_font_from_file(const char *bmp_file_path,
                                            SDL_Renderer *renderer);
void destroy_sprite_font(sprite_font_t *sprite_font);

int sprite_font_render_text(const sprite_font_t *sprite_font,
                            SDL_Renderer *renderer,
                            vec_t position,
                            int size,
                            const char *text);
int sprite_font_debug_render_whole_texture(const sprite_font_t *sprite_font,
                                           SDL_Renderer *renderer,
                                           vec_t position);

#endif  // SPRITE_FONT_H_
