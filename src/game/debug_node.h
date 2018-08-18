#ifndef DEBUG_NODE_H_
#define DEBUG_NODE_H_

typedef enum Debug_node_tag {
    DEBUG_NODE_TAG_N
} Debug_node_tag;

typedef struct Debug_node_ref {
    Debug_node_tag tag;
    void *ptr;
} Debug_node_ref;

const char *debug_node_name(Debug_node_ref debug_node);
Debug_node_ref *debug_node_children(Debug_node_ref debug_node);

#endif  // DEBUG_NODE_H_
