#include <assert.h>

#include "game/sprite_font.h"
#include "system/lt.h"
#include "edit_field.h"

struct edit_field_t
{
    lt_t *lt;
    char *buffer;
    size_t size;
    size_t capacity;
    size_t cursor;
};

edit_field_t *create_edit_field(const sprite_font_t *font)
{
    assert(font);
    return NULL;
}

void destroy_edit_field(edit_field_t *edit_field)
{
    assert(edit_field);
}

int edit_field_render(const edit_field_t *edit_field,
                      SDL_Renderer *renderer,
                      point_t position)
{
    assert(edit_field);
    assert(renderer);
    (void) position;
    return 0;
}

int edit_field_handle_event(edit_field_t *edit_field,
                            const SDL_Event *event)
{
    assert(edit_field);
    assert(event);
    return 0;
}

const char *edit_field_as_text(const edit_field_t *edit_field)
{
    assert(edit_field);
    return NULL;
}
