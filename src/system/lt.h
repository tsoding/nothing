#ifndef LT_H_
#define LT_H_

#define PUSH_LT(lt, resource, resource_destroy) \
    lt_push(lt, (void*)resource, (Lt_destroy)resource_destroy)

#define RESET_LT(lt, old_resource, new_resource)    \
    lt_reset(lt, (void*)old_resource, (void*)new_resource)

#define REPLACE_LT(lt, old_resource, new_resource)  \
    lt_replace(lt, (void*)old_resource, (void*)new_resource)

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

typedef struct Lt Lt;
typedef void (*Lt_destroy)(void*);

Lt *create_lt(void);
void destroy_lt(Lt *lt);

/** \brief Pushes the resource onto the Life Time creating a new Life Time frame.
 */
void *lt_push(Lt *lt, void *resource, Lt_destroy resource_destroy);

/** \brief Destroys old_resource preserving its LT frame and assigns the new_resource to the LT frame.
 */
void *lt_reset(Lt *lt, void *old_resource, void *new_resource);

/** \brief Replaces old_resource with new_resource in the LT frame without destroying old_resource.
 */
void *lt_replace(Lt *lt, void *old_resource, void *new_resource);

/** \brief Removes the LT frame of resource without destroying the resource.
 */
void *lt_release(Lt *lt, void *resource);

#endif  // LT_H_
