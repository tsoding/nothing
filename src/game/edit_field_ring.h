#ifndef EDIT_FIELD_RING_H_
#define EDIT_FIELD_RING_H_

#include "math/point.h"
#include "color.h"

typedef struct EditFieldRing EditFieldRing;
typedef struct Edit_field Edit_field;
typedef struct Sprite_font Sprite_font;

EditFieldRing *create_edit_field_ring(const Sprite_font *font,
                                      Vec font_size,
                                      Color font_color,
                                      size_t capacity);
void destroy_edit_field_ring(EditFieldRing *edit_field_ring);

void edit_field_ring_push(EditFieldRing *edit_field_ring);
size_t edit_field_ring_size(const EditFieldRing *edit_field_ring);
Edit_field *edit_field_ring_get(const EditFieldRing *edit_field_ring, size_t i);

#endif  // EDIT_FIELD_RING_H_
