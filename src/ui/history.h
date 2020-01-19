#ifndef HISTORY_H_
#define HISTORY_H_

typedef struct History History;

History *create_history(size_t capacity);
void destroy_history(History *history);

int history_push(History *history, const char *command);
const char *history_current(History *history);
void history_prev(History *history);
void history_next(History *history);

#endif  // HISTORY_H_
