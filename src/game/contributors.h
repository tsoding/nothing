#ifndef CONTRIBUTORS_H_
#define CONTRIBUTORS_H_

typedef struct Contributors Contributors;

Contributors *create_contributors(void);
void destroy_contributors(Contributors *contributors);

const char **contributors_names(const Contributors *contributors);
size_t contributors_count(const Contributors *contributors);

#endif  // CONTRIBUTORS_H_
