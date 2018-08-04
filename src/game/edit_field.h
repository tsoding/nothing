#ifndef EDIT_FIELD_H_
#define EDIT_FIELD_H_

#include <SDL2/SDL.h>

#include "color.h"
#include "math/point.h"

typedef struct edit_field_t edit_field_t;
typedef struct sprite_font_t sprite_font_t;

edit_field_t *create_edit_field(const sprite_font_t *font,
                                vec_t font_size,
                                color_t font_color);
void destroy_edit_field(edit_field_t *edit_field);

int edit_field_render(const edit_field_t *edit_field,
                      SDL_Renderer *renderer,
                      point_t position);

int edit_field_handle_event(edit_field_t *edit_field,
                            const SDL_Event *event);

const char *edit_field_as_text(const edit_field_t *edit_field);

#endif  // EDIT_FIELD_H_
