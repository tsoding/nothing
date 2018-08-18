#ifndef LT_SLOT_H_
#define LT_SLOT_H_

#include "../lt.h"

typedef struct Lt_slot Lt_slot;

Lt_slot *create_lt_slot(void *resource, Lt_destroy resource_destroy);
void *release_lt_slot(Lt_slot *lt_slot);
void destroy_lt_slot(Lt_slot *lt_slot);

void lt_slot_reset_resource(Lt_slot *lt_slot, void *resource);
void lt_slot_replace_resource(Lt_slot *lt_slot, void *resource);
int lt_slot_contains_resource(const Lt_slot *lt_slot, void *resource);

#endif  // LT_SLOT_H_
