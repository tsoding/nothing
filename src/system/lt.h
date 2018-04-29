#ifndef LT_H_
#define LT_H_

#define PUSH_LT(lt, resource, resource_destroy) \
    lt_push(lt, (void*)resource, (lt_destroy_t)resource_destroy)

#define RESET_LT(lt, old_resource, new_resource)    \
    lt_reset(lt, (void*)old_resource, (void*)new_resource)

#define RELEASE_LT(lt, resource)                \
    lt_release(lt, (void*) resource)

#define RETURN_LT(lt, result)               \
    do {                                    \
        destroy_lt(lt);                     \
        return result;                      \
    } while (0)

#define RETURN_LT0(lt)                          \
    do {                                        \
        destroy_lt(lt);                         \
        return;                                 \
    } while (0)

typedef struct lt_t lt_t;
typedef void (*lt_destroy_t)(void*);

lt_t *create_lt(void);
void destroy_lt(lt_t *lt);

void *lt_push(lt_t *lt, void *resource, lt_destroy_t resource_destroy);
void *lt_reset(lt_t *lt, void *old_resource, void *new_resource);
void *lt_release(lt_t *lt, void *resource);

#endif  // LT_H_
