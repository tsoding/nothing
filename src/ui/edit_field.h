#ifndef EDIT_FIELD_H_
#define EDIT_FIELD_H_

#include <SDL.h>

#include "color.h"
#include "math/vec.h"
#include "game/camera.h"

typedef struct {
    char buffer[EDIT_FIELD_CAPACITY];
    size_t buffer_size;
    size_t cursor;
    Vec2f font_size;
    Color font_color;
} Edit_field;

int edit_field_render_screen(const Edit_field *edit_field,
                             const Camera *camera,
                             Vec2f screen_position);

int edit_field_render_world(const Edit_field *edit_field,
                            const Camera *camera,
                            Vec2f world_position);

int edit_field_event(Edit_field *edit_field, const SDL_Event *event);

const char *edit_field_as_text(const Edit_field *edit_field);

void edit_field_replace(Edit_field *edit_field, const char *text);
void edit_field_clean(Edit_field *edit_field);
void edit_field_restyle(Edit_field *edit_field,
                        Vec2f font_size,
                        Color font_color);

#endif  // EDIT_FIELD_H_
