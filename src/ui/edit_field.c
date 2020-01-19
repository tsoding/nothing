#include <stdbool.h>
#include <string.h>

#include "edit_field.h"
#include "game/camera.h"
#include "game/sprite_font.h"
#include "sdl/renderer.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/stacktrace.h"


static void edit_field_insert_char(Edit_field *edit_field, char c);

// See: https://www.gnu.org/software/emacs/manual/html_node/emacs/Moving-Point.html
// For an explanation of the naming terminology for these helper methods
static bool is_emacs_word(char c);
static void forward_char(Edit_field *edit_field);
static void backward_char(Edit_field *edit_field);
static void move_beginning_of_line(Edit_field *edit_field);
static void move_end_of_line(Edit_field *edit_field);
static void forward_word(Edit_field *edit_field);
static void backward_word(Edit_field *edit_field);
static void kill_region_and_move_cursor(Edit_field *edit_field, size_t start, size_t end);
static void delete_char(Edit_field *edit_field);
static void delete_backward_char(Edit_field *edit_field);
static void kill_word(Edit_field *edit_field);
static void backward_kill_word(Edit_field *edit_field);
static void kill_to_end_of_line(Edit_field *edit_field);
static void field_buffer_cut(Edit_field *edit_field);
static void field_buffer_copy(Edit_field *edit_field);
static void field_buffer_paste(Edit_field *edit_field);

static void handle_keydown(Edit_field *edit_field, const SDL_Event *event);
static void handle_keydown_alt(Edit_field *edit_field, const SDL_Event *event);
static void handle_keydown_ctrl(Edit_field *edit_field, const SDL_Event *event);

static void edit_field_insert_char(Edit_field *edit_field, char c)
{
    if (edit_field->buffer_size >= EDIT_FIELD_CAPACITY) {
        return;
    }

    char *dest = edit_field->buffer + edit_field->cursor + 1;
    memmove(dest, dest - 1, edit_field->buffer_size - edit_field->cursor);

    edit_field->buffer[edit_field->cursor++] = c;
    edit_field->buffer[++edit_field->buffer_size] = 0;
}

static inline
void edit_field_insert_string(Edit_field *edit_field, const char *text)
{
    size_t n = strlen(text);
    for (size_t i = 0; i < n; ++i) {
        edit_field_insert_char(edit_field, text[i]);
    }
}

// See: https://www.gnu.org/software/emacs/manual/html_node/emacs/Moving-Point.html
// For an explanation of the naming terminology for these helper methods

static bool is_emacs_word(char c)
{
    // Word syntax table retrieved from Fundamental Mode, "C-h s"
    // (This is not the complete syntax table)
    return (c >= '$' && c <= '%')
        || (c >= '0' && c <= '9')
        || (c >= 'A' && c <= 'Z')
        || (c >= 'a' && c <= 'z');
}

static void forward_char(Edit_field *edit_field)
{
    // "C-f" or "<RIGHT>"
    if (edit_field->cursor < edit_field->buffer_size) {
        edit_field->cursor++;
    }
}

static void backward_char(Edit_field *edit_field)
{
    // "C-b" or "<LEFT>"
    if (edit_field->cursor > 0) {
        edit_field->cursor--;
    }
}

static void move_beginning_of_line(Edit_field *edit_field)
{
    // "C-a" or "<Home>"
    edit_field->cursor = 0;
}

static void move_end_of_line(Edit_field *edit_field)
{
    // "C-e" or "<End>"
    edit_field->cursor = edit_field->buffer_size;
}

static void forward_word(Edit_field *edit_field)
{
    // "M-f" or "C-<RIGHT>" or "M-<RIGHT>"
    while (true) {
        forward_char(edit_field);
        if (edit_field->cursor >= edit_field->buffer_size) {
            break;
        }

        char current = edit_field->buffer[edit_field->cursor];
        char preceeding = edit_field->buffer[edit_field->cursor - 1];
        if (!is_emacs_word(current) && is_emacs_word(preceeding)) {
            // Reached the end of the current word
            break;
        }
    }
}

static void backward_word(Edit_field *edit_field)
{
    // "M-b" or "C-<LEFT>" or "M-<LEFT>"
    while (true) {
        backward_char(edit_field);
        if (edit_field->cursor == 0) {
            break;
        }

        char current = edit_field->buffer[edit_field->cursor];
        char preceeding = edit_field->buffer[edit_field->cursor - 1];
        if (is_emacs_word(current) && !is_emacs_word(preceeding)) {
            // Reached the start of the current word
            break;
        }
    }
}

static void kill_region_and_move_cursor(Edit_field *edit_field, size_t start, size_t end) {
    trace_assert(end <= edit_field->buffer_size);

    if (end <= start) {
        // Nothing to delete
        return;
    }

    size_t to_delete = end - start;
    size_t to_move = edit_field->buffer_size - end;

    if (to_move > 0) {
        char *dest = edit_field->buffer + start;
        memmove(dest, dest + to_delete, to_move);
    }

    edit_field->buffer[start + to_move] = 0;
    edit_field->buffer_size -= to_delete;

    edit_field->cursor = start;
}

static void delete_char(Edit_field *edit_field)
{
    // "C-d" or "<Delete>"
    if (edit_field->cursor >= edit_field->buffer_size) {
        return;
    }

    kill_region_and_move_cursor(edit_field, edit_field->cursor, edit_field->cursor + 1);
}

static void delete_backward_char(Edit_field *edit_field)
{
    // "<BACKSPACE>"
    if (edit_field->cursor == 0) {
        return;
    }

    kill_region_and_move_cursor(edit_field, edit_field->cursor - 1, edit_field->cursor);
}

static void kill_word(Edit_field *edit_field)
{
    // "M-d" or "C-<Delete>"
    size_t start = edit_field->cursor;
    forward_word(edit_field);
    size_t end = edit_field->cursor;

    kill_region_and_move_cursor(edit_field, start, end);
}

static void backward_kill_word(Edit_field *edit_field)
{
    // "M-<BACKSPACE>" or "C-<BACKSPACE>" or "M-<Delete>"
    size_t end = edit_field->cursor;
    backward_word(edit_field);
    size_t start = edit_field->cursor;

    kill_region_and_move_cursor(edit_field, start, end);
}

static void kill_to_end_of_line(Edit_field *edit_field) {
    // "C-k"
    kill_region_and_move_cursor(edit_field, edit_field->cursor,
                                edit_field->buffer_size);
}

static void field_buffer_cut(Edit_field *edit_field) {
    // "C-w"
    SDL_SetClipboardText(edit_field_as_text(edit_field));
    edit_field_clean(edit_field);
}

static void field_buffer_copy(Edit_field *edit_field) {
    // "M-w"
    SDL_SetClipboardText(edit_field_as_text(edit_field));
}

static void field_buffer_paste(Edit_field *edit_field) {
    // "C-y"
    char *text = SDL_GetClipboardText();
    edit_field_insert_string(edit_field, text);
}

static void handle_keydown(Edit_field *edit_field, const SDL_Event *event)
{
    switch (event->key.keysym.sym) {
    case SDLK_HOME: {
        move_beginning_of_line(edit_field);
    } break;

    case SDLK_END: {
        move_end_of_line(edit_field);
    } break;

    case SDLK_BACKSPACE: {
        delete_backward_char(edit_field);
    } break;

    case SDLK_DELETE: {
        delete_char(edit_field);
    } break;

    case SDLK_RIGHT: {
        forward_char(edit_field);
    } break;

    case SDLK_LEFT: {
        backward_char(edit_field);
    } break;
    }
}

static void handle_keydown_alt(Edit_field *edit_field, const SDL_Event *event)
{
    switch (event->key.keysym.sym) {
    case SDLK_BACKSPACE:
    case SDLK_DELETE: {
        backward_kill_word(edit_field);
    } break;

    case SDLK_RIGHT:
    case SDLK_f: {
        forward_word(edit_field);
    } break;

    case SDLK_LEFT:
    case SDLK_b: {
        backward_word(edit_field);
    } break;

    case SDLK_d: {
        kill_word(edit_field);
    } break;

    // TODO(#1220): edit_field doesn't support selections for copy/cut operations
    case SDLK_w: {
        field_buffer_copy(edit_field);
    } break;
    }
}

static void handle_keydown_ctrl(Edit_field *edit_field, const SDL_Event *event)
{
    switch (event->key.keysym.sym) {
    case SDLK_BACKSPACE: {
        backward_kill_word(edit_field);
    } break;

    case SDLK_DELETE: {
        kill_word(edit_field);
    } break;

    case SDLK_RIGHT: {
        forward_word(edit_field);
    } break;

    case SDLK_LEFT: {
        backward_word(edit_field);
    } break;

    case SDLK_a: {
        move_beginning_of_line(edit_field);
    } break;

    case SDLK_e: {
        move_end_of_line(edit_field);
    } break;

    case SDLK_f: {
        forward_char(edit_field);
    } break;

    case SDLK_b: {
        backward_char(edit_field);
    } break;

    case SDLK_d: {
        delete_char(edit_field);
    } break;

    case SDLK_k: {
        kill_to_end_of_line(edit_field);
    } break;

    case SDLK_w:
    case SDLK_x: {
        field_buffer_cut(edit_field);
    } break;

    case SDLK_y:
    case SDLK_v: {
        field_buffer_paste(edit_field);
    } break;

    case SDLK_c: {
        field_buffer_copy(edit_field);
    } break;
    }
}

int edit_field_render_screen(const Edit_field *edit_field,
                             const Camera *camera,
                             Vec2f screen_position)
{
    trace_assert(edit_field);
    trace_assert(camera);

    const float cursor_y_overflow = 10.0f;
    const float cursor_width = 2.0f;

    camera_render_text_screen(
        camera,
        edit_field->buffer,
        edit_field->font_size,
        edit_field->font_color,
        screen_position);

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
                            const Camera *camera,
                            Vec2f world_position)
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

int edit_field_event(Edit_field *edit_field, const SDL_Event *event)
{
    trace_assert(edit_field);
    trace_assert(event);

    switch (event->type) {
    case SDL_KEYDOWN: {
        if (event->key.keysym.mod & KMOD_ALT) {
            handle_keydown_alt(edit_field, event);
        } else if (event->key.keysym.mod & KMOD_CTRL) {
            handle_keydown_ctrl(edit_field, event);
        } else {
            handle_keydown(edit_field, event);
        }
    } break;

    case SDL_TEXTINPUT: {
        if ((SDL_GetModState() & (KMOD_CTRL | KMOD_ALT))) {
            // Don't process text input if a modifier key is held
            break;
        }

        size_t n = strlen(event->text.text);
        for (size_t i = 0; i < n; ++i) {
            edit_field_insert_char(edit_field, event->text.text[i]);
        }
    } break;
    }

    return 0;
}

const char *edit_field_as_text(const Edit_field *edit_field)
{
    trace_assert(edit_field);
    return edit_field->buffer;
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

void edit_field_clean(Edit_field *edit_field)
{
    trace_assert(edit_field);

    edit_field->cursor = 0;
    edit_field->buffer_size = 0;
    edit_field->buffer[0] = 0;
}

void edit_field_restyle(Edit_field *edit_field,
                        Vec2f font_size,
                        Color font_color)
{
    trace_assert(edit_field);
    edit_field->font_size = font_size;
    edit_field->font_color = font_color;
}
