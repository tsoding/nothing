#include <assert.h>

#include "edit_field.h"
#include "game/sprite_font.h"
#include "system/error.h"
#include "system/lt.h"

#define EDIT_FIELD_BUFFER_INIT_SIZE 256

struct edit_field_t
{
    lt_t *lt;
    char *buffer;
    size_t size;
    size_t capacity;
    size_t cursor;
    const sprite_font_t *font;
};

edit_field_t *create_edit_field(const sprite_font_t *font)
{
    assert(font);

    lt_t *lt = create_lt();

    if (lt == NULL) {
        return NULL;
    }

    edit_field_t *const edit_field = PUSH_LT(lt, malloc(sizeof(edit_field_t)), free);
    if (edit_field == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    edit_field->lt = lt;

    edit_field->buffer = PUSH_LT(lt, malloc(sizeof(char) * EDIT_FIELD_BUFFER_INIT_SIZE), free);
    if (edit_field->buffer == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    edit_field->size = 0;
    edit_field->capacity = EDIT_FIELD_BUFFER_INIT_SIZE;
    edit_field->cursor = 0;
    edit_field->font = font;

    return edit_field;
}

void destroy_edit_field(edit_field_t *edit_field)
{
    assert(edit_field);
    RETURN_LT0(edit_field->lt);
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
