#include <stdio.h>
#include <stdlib.h>
#include "system/stacktrace.h"

#include "history.h"
#include "system/str.h"
#include "system/lt.h"
#include "system/nth_alloc.h"

struct History
{
    Lt *lt;

    char **buffer;
    size_t begin;
    size_t capacity;
    size_t cursor;
};

History *create_history(size_t capacity)
{
    Lt *lt = create_lt();

    History *history = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(History)),
        free);
    if (history == NULL) {
        RETURN_LT(lt, NULL);
    }
    history->lt = lt;

    history->capacity = capacity;
    history->begin = 0;
    history->cursor = 0;

    history->buffer = PUSH_LT(lt, nth_calloc(capacity, sizeof(char*)), free);
    if (history->buffer == NULL) {
        RETURN_LT(lt, NULL);
    }

    return history;
}

void destroy_history(History *history)
{
    trace_assert(history);

    for (size_t i = 0; i < history->capacity; ++i) {
        if (history->buffer[i] != NULL) {
            free(history->buffer[i]);
        }
    }

    RETURN_LT0(history->lt);
}

int history_push(History *history, const char *command)
{
    trace_assert(history);
    trace_assert(command);

    const size_t next_begin = (history->begin + 1) % history->capacity;

    if (history->buffer[history->begin] != NULL) {
        free(history->buffer[history->begin]);
    }

    history->buffer[history->begin] = string_duplicate(command, NULL);

    if (history->buffer[history->begin] == NULL) {
        return -1;
    }

    history->begin = next_begin;
    history->cursor = next_begin;

    return 0;
}

const char *history_current(History *history)
{
    trace_assert(history);
    return history->buffer[history->cursor];
}

void history_prev(History *history)
{
    trace_assert(history);
    if (history->cursor == 0) {
        history->cursor = history->capacity - 1;
    } else {
        history->cursor--;
    }
}

void history_next(History *history)
{
    trace_assert(history);
    history->cursor = (history->cursor + 1) % history->capacity;
}
