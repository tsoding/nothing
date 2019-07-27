#include <stdbool.h>

#include "edit_field.h"
#include "game/camera.h"
#include "game/sprite_font.h"
#include "sdl/renderer.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/stacktrace.h"

#define BUFFER_CAPACITY 256

struct Edit_field
{
    Lt *lt;
    char *buffer;
    size_t buffer_size;
    size_t cursor;
    Vec font_size;
    Color font_color;
};

static void edit_field_left(Edit_field *edit_field);
static void edit_field_right(Edit_field *edit_field);
static void edit_field_home(Edit_field *edit_field);
static void edit_field_end(Edit_field *edit_field);
static void edit_field_backspace(Edit_field *edit_field);
static void edit_field_delete(Edit_field *edit_field);
static void edit_field_insert_char(Edit_field *edit_field, char c);

Edit_field *create_edit_field(Vec font_size,
                              Color font_color)
{
    Lt *lt = create_lt();

    Edit_field *const edit_field = PUSH_LT(lt, nth_calloc(1, sizeof(Edit_field)), free);
    if (edit_field == NULL) {
        RETURN_LT(lt, NULL);
    }
    edit_field->lt = lt;

    edit_field->buffer = PUSH_LT(lt, nth_calloc(1, sizeof(char) * (BUFFER_CAPACITY + 10)), free);
    if (edit_field->buffer == NULL) {
        RETURN_LT(lt, NULL);
    }

    edit_field->buffer_size = 0;
    edit_field->cursor = 0;
    edit_field->font_size = font_size;
    edit_field->font_color = font_color;

    edit_field->buffer[edit_field->buffer_size] = 0;

    return edit_field;
}

void destroy_edit_field(Edit_field *edit_field)
{
    trace_assert(edit_field);
    RETURN_LT0(edit_field->lt);
}

int edit_field_render_screen(const Edit_field *edit_field,
                             Camera *camera,
                             Point screen_position)
{
    trace_assert(edit_field);
    trace_assert(camera);

    const float cursor_y_overflow = 10.0f;
    const float cursor_width = 2.0f;

    if (camera_render_text_screen(
            camera,
            edit_field->buffer,
            edit_field->font_size,
            edit_field->font_color,
            screen_position) < 0) {
        return -1;
    }

    /* TODO(#363): the size of the cursor does not correspond to font size */
    if (camera_fill_rect_screen(
            camera,
            rect(screen_position.x + (float) edit_field->cursor * (float) FONT_CHAR_WIDTH * edit_field->font_size.x,
                 screen_position.y - cursor_y_overflow,
                 cursor_width,
                 FONT_CHAR_HEIGHT * edit_field->font_size.y + cursor_y_overflow * 2.0f),
            edit_field->font_color) < 0) {
        return -1;
    }

    return 0;
}

int edit_field_render_world(const Edit_field *edit_field,
                            Camera *camera,
                            Point world_position)
{
    trace_assert(edit_field);
    trace_assert(camera);

    const float cursor_y_overflow = 10.0f;
    const float cursor_width = 2.0f;

    if (camera_render_text(
            camera,
            edit_field->buffer,
            edit_field->font_size,
            edit_field->font_color,
            world_position) < 0) {
        return -1;
    }

    if (camera_fill_rect(
            camera,
            rect(world_position.x + (float) edit_field->cursor * (float) FONT_CHAR_WIDTH * edit_field->font_size.x,
                 world_position.y - cursor_y_overflow,
                 cursor_width,
                 FONT_CHAR_HEIGHT * edit_field->font_size.y + cursor_y_overflow * 2.0f),
            edit_field->font_color) < 0) {
        return -1;
    }

    return 0;
}

const char *edit_field_as_text(const Edit_field *edit_field)
{
    trace_assert(edit_field);
    return edit_field->buffer;
}

static void edit_field_left(Edit_field *edit_field)
{
    trace_assert(edit_field);
    if (edit_field->cursor > 0) {
        edit_field->cursor--;
    }
}

static void edit_field_home(Edit_field *edit_field)
{
    trace_assert(edit_field);
    edit_field->cursor = 0;
}

static void edit_field_end(Edit_field *edit_field)
{
    trace_assert(edit_field);
    edit_field->cursor  = edit_field->buffer_size;
}

static void edit_field_right(Edit_field *edit_field)
{
    trace_assert(edit_field);
    if (edit_field->cursor < edit_field->buffer_size) {
        edit_field->cursor++;
    }
}

static void edit_field_backspace(Edit_field *edit_field)
{
    trace_assert(edit_field);

    if (edit_field->cursor == 0) {
        return;
    }

    for (size_t i = edit_field->cursor; i < edit_field->buffer_size; ++i) {
        edit_field->buffer[i - 1] = edit_field->buffer[i];
    }

    edit_field->cursor--;
    edit_field->buffer[--edit_field->buffer_size] = 0;
}

static void edit_field_delete(Edit_field *edit_field)
{
    trace_assert(edit_field);

    if (edit_field->cursor >= edit_field->buffer_size) {
        return;
    }

    for (size_t i = edit_field->cursor; i < edit_field->buffer_size; ++i) {
        edit_field->buffer[i] = edit_field->buffer[i + 1];
    }

    edit_field->buffer[--edit_field->buffer_size] = 0;
}

static void edit_field_insert_char(Edit_field *edit_field, char c)
{
    trace_assert(edit_field);

    if (edit_field->buffer_size >= BUFFER_CAPACITY) {
        return;
    }

    for (int64_t i = (int64_t) edit_field->buffer_size - 1; i >= (int64_t) edit_field->cursor; --i) {
        edit_field->buffer[i + 1] = edit_field->buffer[i];
    }

    edit_field->buffer[edit_field->cursor++] = c;
    edit_field->buffer[++edit_field->buffer_size] = 0;
}

void edit_field_clean(Edit_field *edit_field)
{
    trace_assert(edit_field);

    edit_field->cursor = 0;
    edit_field->buffer_size = 0;
    edit_field->buffer[0] = 0;
}

void edit_field_replace(Edit_field *edit_field, const char *text)
{
    trace_assert(edit_field);

    edit_field_clean(edit_field);

    if (text == NULL) {
        return;
    }

    // TODO(#983): edit_field_replace should probably use memcpy
    size_t n = strlen(text);
    for (size_t i = 0; i < n; ++i) {
        edit_field_insert_char(edit_field, text[i]);
    }
}

int edit_field_event(Edit_field *edit_field, const SDL_Event *event)
{
    trace_assert(edit_field);
    trace_assert(event);

    switch (event->type) {
    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_LEFT:
            edit_field_left(edit_field);
            break;

        case SDLK_RIGHT:
            edit_field_right(edit_field);
            break;

        case SDLK_HOME:
            edit_field_home(edit_field);
            break;

        case SDLK_END:
            edit_field_end(edit_field);
            break;

        case SDLK_BACKSPACE:
            edit_field_backspace(edit_field);
            break;

        case SDLK_DELETE:
            edit_field_delete(edit_field);
            break;
        }
    } break;

    case SDL_TEXTINPUT: {
        size_t n = strlen(event->text.text);
        for (size_t i = 0; i < n; ++i) {
            edit_field_insert_char(edit_field, event->text.text[i]);
        }
    } break;
    }

    return 0;
}

void edit_field_restyle(Edit_field *edit_field,
                        Vec font_size,
                        Color font_color)
{
    trace_assert(edit_field);
    edit_field->font_size = font_size;
    edit_field->font_color = font_color;
}
