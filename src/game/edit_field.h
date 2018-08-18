#ifndef EDIT_FIELD_H_
#define EDIT_FIELD_H_

#include <SDL2/SDL.h>

#include "color.h"
#include "math/point.h"

typedef struct Edit_field Edit_field;
typedef struct Sprite_font Sprite_font;

Edit_field *create_edit_field(const Sprite_font *font,
                                Vec font_size,
                                Color font_color);
void destroy_edit_field(Edit_field *edit_field);

int edit_field_render(const Edit_field *edit_field,
                      SDL_Renderer *renderer,
                      Point position);

int edit_field_handle_event(Edit_field *edit_field,
                            const SDL_Event *event);

const char *edit_field_as_text(const Edit_field *edit_field);

#endif  // EDIT_FIELD_H_
