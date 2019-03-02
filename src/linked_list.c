#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "system/stacktrace.h"
#include "system/nth_alloc.h"

#include "linked_list.h"

struct LinkedList
{
    size_t element_size;
    NodeLL *last;
};

LinkedList *create_linked_list(size_t element_size)
{
    LinkedList *linked_list = nth_alloc(sizeof(LinkedList));
    if (linked_list == NULL) {
        return NULL;
    }

    linked_list->element_size = element_size;
    linked_list->last = NULL;

    return linked_list;
}

void destroy_linked_list(LinkedList *linked_list)
{
    trace_assert(linked_list);

    while (!linked_list_empty(linked_list)) {
        linked_list_pop_back(linked_list);;
    }

    free(linked_list);
}

NodeLL *linked_list_push_back(LinkedList *linked_list,
                              const void *element)
{
    trace_assert(linked_list);
    trace_assert(element);

    NodeLL *noodle = nth_alloc(sizeof(NodeLL) + linked_list->element_size);
    if (noodle == NULL) {
        return NULL;
    }

    memcpy(noodle->data, element, linked_list->element_size);
    noodle->prev = NULL;
    noodle->next = NULL;

    if (linked_list->last == NULL) {
        linked_list->last = noodle;
    } else {
        linked_list->last->next = noodle;
        noodle->prev = linked_list->last;
        linked_list->last = noodle;
    }

    return noodle;
}

void linked_list_pop_back(LinkedList *linked_list)
{
    trace_assert(linked_list);

    if (linked_list->last == NULL) {
        return;
    }

    NodeLL *noodle = linked_list->last;
    linked_list->last = noodle->prev;
    free(noodle);
}

bool linked_list_empty(const LinkedList *linked_list)
{
    return linked_list->last == NULL;
}

NodeLL *linked_list_find(const LinkedList *linked_list,
                         const void *element)
{
    trace_assert(linked_list);
    trace_assert(element);

    NodeLL *node = linked_list->last;
    while (node != NULL && memcmp(element, node->data, linked_list->element_size) != 0) {
        node = node->prev;
    }

    return node;
}

void linked_list_remove(LinkedList *linked_list,
                        NodeLL *noodle)
{
    trace_assert(linked_list);
    trace_assert(noodle);

    if (linked_list->last == noodle) {
        linked_list->last = linked_list->last->prev;
    }

    if (noodle->prev != NULL) {
        noodle->prev->next = noodle->next;
    }

    if (noodle->next != NULL) {
        noodle->next->prev = noodle->prev;
    }

    free(noodle);
}

NodeLL *linked_list_last(const LinkedList *linked_list)
{
    return linked_list->last;
}
