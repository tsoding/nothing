#ifndef LT_H_
#define LT_H_

#define LT_PUSH(lt, resource, resource_destroy) \
    lt_push(lt, (void*)resource, (lt_destroy_t)resource_destroy)

#define LT_RESET(lt, old_resource, new_resource)    \
    lt_reset(lt, (void*)old_resource, (void*)new_resource)

#define LT_RETURN(lt, ...)                  \
    destroy_lt(lt);                         \
    return __VA_ARGS__

typedef struct lt_t lt_t;
typedef void (*lt_destroy_t)(void*);

lt_t *create_lt(void);
void destroy_lt(lt_t *lt);

void *lt_push(lt_t *lt, void *resource, lt_destroy_t resource_destroy);
void *lt_reset(lt_t *lt, void *old_resource, void *new_resource);

#endif  // LT_H_
