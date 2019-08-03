#ifndef UNDO_HISTORY_H_
#define UNDO_HISTORY_H_

#include "layer.h"

typedef struct {
    LayerPtr layer;
} Action;

typedef struct UndoHistory UndoHistory;

UndoHistory *create_undo_history(void);
void destroy_undo_history(UndoHistory *undo_history);

void undo_history_push(UndoHistory *undo_history, Action action);
void undo_history_pop(UndoHistory *undo_history);

#endif  // UNDO_HISTORY_H_
