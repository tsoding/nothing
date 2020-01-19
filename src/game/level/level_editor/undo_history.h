#ifndef UNDO_HISTORY_H_
#define UNDO_HISTORY_H_

#include "ring_buffer.h"

typedef void (*RevertAction)(void *context, size_t context_size);

typedef struct {
    RingBuffer actions;
    Memory *memory;
} UndoHistory;

UndoHistory *create_undo_history(Memory *memory);

void undo_history_push(UndoHistory *undo_history,
                       RevertAction revert,
                       void *context_data,
                       size_t context_data_size);
void undo_history_pop(UndoHistory *undo_history);

void undo_history_clean(UndoHistory *undo_history);

static inline
int undo_history_empty(UndoHistory *undo_history)
{
    return undo_history->actions.count == 0;
}

#endif  // UNDO_HISTORY_H_
