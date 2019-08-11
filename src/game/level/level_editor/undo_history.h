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

typedef struct UndoHistory UndoHistory;

UndoHistory *create_undo_history(void);
void destroy_undo_history(UndoHistory *undo_history);

void undo_history_push(UndoHistory *undo_history, Action action);
void undo_history_pop(UndoHistory *undo_history);

#endif  // UNDO_HISTORY_H_
