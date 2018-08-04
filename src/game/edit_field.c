#include <assert.h>

#include "edit_field.h"
#include "game/sprite_font.h"
#include "sdl/renderer.h"
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

    const char *test_text = "Hello World";

    strcpy(edit_field->buffer, test_text);
    edit_field->size = strlen(test_text);
    edit_field->capacity = EDIT_FIELD_BUFFER_INIT_SIZE;
    edit_field->cursor = 5;
    edit_field->font = font;

    edit_field->buffer[edit_field->size] = 0;

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

    const vec_t size = vec(5.0f, 5.0f);
    const color_t field_color = color(1.0f, 1.0f, 1.0f, 1.0f);
    const float cursor_y_overflow = 10.0f;
    const float cursor_width = 2.0f;

    /* TODO: size and color of edit_field should be customizable */
    if (sprite_font_render_text(edit_field->font,
                                renderer,
                                position,
                                size,
                                field_color,
                                edit_field->buffer) < 0) {
        return -1;
    }

    if (fill_rect(
            renderer,
            rect(position.x + (float) edit_field->cursor * (float) FONT_CHAR_WIDTH * size.x,
                 position.y - cursor_y_overflow,
                 cursor_width,
                 FONT_CHAR_HEIGHT * size.y + cursor_y_overflow * 2.0f),
            field_color) < 0) {
        return -1;
    }

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
    return edit_field->buffer;
}
