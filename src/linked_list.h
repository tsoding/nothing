#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stdbool.h>

typedef struct LinkedList LinkedList;

typedef struct NodeLL NodeLL;
struct NodeLL
{
    NodeLL *next;
    NodeLL *prev;
    char data[];
};

LinkedList *create_linked_list(size_t element_size);
void destroy_linked_list(LinkedList *linked_list);

NodeLL *linked_list_push_back(LinkedList *linked_list,
                              const void *element);
void linked_list_pop_back(LinkedList *linked_list);
bool linked_list_empty(const LinkedList *linked_list);
NodeLL *linked_list_find(const LinkedList *linked_list,
                               const void *element);
void linked_list_remove(LinkedList *linked_list,
                        NodeLL *noodle);
NodeLL *linked_list_last(const LinkedList *linked_list);
void linked_list_clear(LinkedList *linked_list);

#endif  // LINKED_LIST_H_
