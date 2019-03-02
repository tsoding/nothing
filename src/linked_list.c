#include <stdlib.h>
#include "system/stacktrace.h"
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
    (void) element_size;
    return NULL;
}

void destroy_linked_list(LinkedList *linked_list)
{
    trace_assert(linked_list);
}

void linked_list_push_back(LinkedList *linked_list,
                           void *element)
{
    trace_assert(linked_list);
    trace_assert(element);
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
