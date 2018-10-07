#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "history.h"
#include "system/lt.h"
#include "system/error.h"
#include "str.h"

struct History
{
    Lt *lt;

    char **buffer;
    size_t cursor;
    size_t capacity;
};

History *create_history(size_t capacity)
{
    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    History *history = PUSH_LT(
        lt,
        malloc(sizeof(History)),
        free);
    if (history == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    history->lt = lt;

    history->capacity = capacity;
    history->cursor = 0;

    history->buffer = PUSH_LT(lt, calloc(capacity, sizeof(char*)), free);
    if (history->buffer == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    return history;
}

void destroy_history(History *history)
{
    assert(history);

    for (size_t i = 0; i < history->capacity; ++i) {
        if (history->buffer[i] != NULL) {
            free(history->buffer[i]);
        }
    }

    RETURN_LT0(history->lt);
}

int history_push(History *history, const char *command)
{
    assert(history);
    assert(command);

    const size_t next_cursor = (history->cursor + 1) % history->capacity;

    if (history->buffer[history->cursor] != NULL) {
        free(history->buffer[history->cursor]);
    }

    history->buffer[history->cursor] = string_duplicate(command, NULL);

    if (history->buffer[history->cursor] == NULL) {
        return -1;
    }

    history->cursor = next_cursor;

    return 0;
}

const char *history_get(History *history, size_t i)
{
    assert(history);
    return history->buffer[(history->cursor + i) % history->capacity];
}
