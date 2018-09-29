#include <stdio.h>
#include <assert.h>

#include "edit_field_ring.h"
#include "edit_field.h"
#include "system/lt.h"
#include "system/error.h"
#include "game/sprite_font.h"
#include "game/edit_field.h"

/* TODO(#368): EditFieldRing is not implemented */
struct EditFieldRing
{
    Lt *lt;
    Edit_field *edit_field;
};

EditFieldRing *create_edit_field_ring(const Sprite_font *font,
                                      Vec font_size,
                                      Color font_color,
                                      size_t capacity)
{
    assert(font);
    (void) capacity;

    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    EditFieldRing *edit_field_ring = PUSH_LT(
        lt,
        malloc(sizeof(EditFieldRing)),
        free);
    if (edit_field_ring == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    edit_field_ring->lt = lt;

    edit_field_ring->edit_field = PUSH_LT(
        lt,
        create_edit_field(font, font_size, font_color),
        destroy_edit_field);
    if (edit_field_ring->edit_field == NULL) {
        RETURN_LT(lt, NULL);
    }
    edit_field_focus(edit_field_ring->edit_field);

    return edit_field_ring;
}

void destroy_edit_field_ring(EditFieldRing *edit_field_ring)
{
    assert(edit_field_ring);
    RETURN_LT0(edit_field_ring->lt);
}

void edit_field_ring_push(EditFieldRing *edit_field_ring)
{
    assert(edit_field_ring);
    edit_field_clean(edit_field_ring->edit_field);
}

size_t edit_field_ring_size(const EditFieldRing *edit_field_ring)
{
    assert(edit_field_ring);
    return 1;
}

Edit_field *edit_field_ring_get(const EditFieldRing *edit_field_ring, size_t i)
{
    assert(edit_field_ring);
    (void) i;
    return edit_field_ring->edit_field;
}
