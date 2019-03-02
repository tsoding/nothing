#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

typedef struct LinkedList LinkedList;
typedef struct NodeLL NodeLL;

LinkedList *create_linked_list(size_t element_size);
void destroy_linked_list(LinkedList *linked_list);

NodeLL *linked_list_push_back(LinkedList *linked_list,
                              void *element);
void linked_list_pop_back(LinkedList *linked_list);
bool linked_list_empty(const LinkedList *linked_list);
NodeLL *linked_list_find(const LinkedList *linked_list,
                               void *element);
void linked_list_remove(LinkedList *linked_list,
                        NodeLL *noodle);

#endif  // LINKED_LIST_H_
