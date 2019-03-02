#include <stdlib.h>
#include <stdbool.h>
#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "linked_list.h"

struct LinkedList
{
    size_t element_size;
    NodeLL *head;
};

struct NodeLL
{
    void *data;
    NodeLL *next;
};

LinkedList *create_linked_list(size_t element_size)
{
    LinkedList *linked_list = nth_alloc(sizeof(LinkedList));
    if (linked_list == NULL) {
        return NULL;
    }

    linked_list->element_size = element_size;
    linked_list->head = NULL;

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

void linked_list_push_back(LinkedList *linked_list,
                           void *element)
{
    trace_assert(linked_list);
    trace_assert(element);
}

void linked_list_pop_back(LinkedList *linked_list)
{
    trace_assert(linked_list);
}

bool linked_list_empty(const LinkedList *linked_list)
{
    return linked_list->head == NULL;
}

NodeLL *linked_list_find(const LinkedList *linked_list,
                         void *element)
{
    trace_assert(linked_list);
    trace_assert(element);

    return NULL;
}

void linked_list_remove(LinkedList *linked_list,
                        NodeLL *noodle)
{
    trace_assert(linked_list);
    trace_assert(noodle);
}
