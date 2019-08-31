#ifndef UNDO_HISTORY_H_
#define UNDO_HISTORY_H_

#include "stack.h"

typedef void (*RevertAction)(void *layer, void *context, size_t context_size);

typedef struct {
    Stack actions;
} UndoHistory;

static inline
void destroy_undo_history(UndoHistory undo_history)
{
    destroy_stack(undo_history.actions);
}

void undo_history_push(UndoHistory *undo_history,
                       void *layer,
                       RevertAction revert,
                       void *context_data,
                       size_t context_data_size);
void undo_history_pop(UndoHistory *undo_history);

#endif  // UNDO_HISTORY_H_
