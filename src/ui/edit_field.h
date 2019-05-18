#ifndef EDIT_FIELD_H_
#define EDIT_FIELD_H_

#include <SDL2/SDL.h>

#include "color.h"
#include "math/point.h"

typedef struct Edit_field Edit_field;
typedef struct Camera Camera;

Edit_field *create_edit_field(Vec font_size,
                              Color font_color);
void destroy_edit_field(Edit_field *edit_field);

int edit_field_render(const Edit_field *edit_field,
                      Camera *camera,
                      Point screen_position);

int edit_field_handle_event(Edit_field *edit_field,
                            const SDL_Event *event);

const char *edit_field_as_text(const Edit_field *edit_field);

void edit_field_replace(Edit_field *edit_field, const char *text);
void edit_field_clean(Edit_field *edit_field);

#endif  // EDIT_FIELD_H_
