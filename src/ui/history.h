#ifndef HISTORY_H_
#define HISTORY_H_

typedef struct History History;

History *create_history(size_t capacity);
void destroy_history(History *history);

int history_push(History *history, const char *command);
const char *history_get(History *history, size_t i);

#endif  // HISTORY_H_
