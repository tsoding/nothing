#ifndef ACTION_H_
#define ACTION_H_

#include "config.h"

typedef enum {
    ACTION_NONE = 0,
    ACTION_HIDE_LABEL,
    ACTION_TOGGLE_GOAL,

    ACTION_N
} ActionType;

typedef struct {
    ActionType type;
    char entity_id[ENTITY_MAX_ID_SIZE];
} Action;

#endif  // ACTION_H_
