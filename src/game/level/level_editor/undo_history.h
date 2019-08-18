#ifndef UNDO_HISTORY_H_
#define UNDO_HISTORY_H_

#include "layer.h"

#define CONTEXT_SIZE 256

#define ASSERT_CONTEXT_SIZE(context)               \
    trace_assert(sizeof(context) <= CONTEXT_SIZE)

typedef struct {
    char data[CONTEXT_SIZE];
} Context;

typedef void (*RevertAction)(void *layer, Context context);

typedef struct {
    void *layer;
    Context context;
    RevertAction revert;
} Action;

static inline
Action create_action(void *layer, RevertAction revert,
                     void *context_data,
                     size_t context_data_size)
{
    trace_assert(layer);
    trace_assert(revert);
    trace_assert(context_data_size < CONTEXT_SIZE);

    Action action = {
        .layer = layer,
        .revert = revert
    };

    if (context_data) {
        memcpy(action.context.data, context_data, context_data_size);
    }

    return action;
}

typedef struct UndoHistory UndoHistory;

UndoHistory *create_undo_history(void);
void destroy_undo_history(UndoHistory *undo_history);

void undo_history_push(UndoHistory *undo_history, Action action);
void undo_history_pop(UndoHistory *undo_history);

#endif  // UNDO_HISTORY_H_
