#ifndef LT_SLOT_H_
#define LT_SLOT_H_

#include "../lt.h"

typedef struct lt_slot_t lt_slot_t;

lt_slot_t *create_lt_slot(void *resource, lt_destroy_t resource_destroy);
void *release_lt_slot(lt_slot_t *lt_slot);
void destroy_lt_slot(lt_slot_t *lt_slot);

void lt_slot_reset_resource(lt_slot_t *lt_slot, void *resource);
int lt_slot_contains_resource(const lt_slot_t *lt_slot, void *resource);

#endif  // LT_SLOT_H_
