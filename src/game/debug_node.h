#ifndef DEBUG_NODE_H_
#define DEBUG_NODE_H_

typedef enum debug_node_tag_t {
    DEBUG_NODE_TAG_N
} debug_node_tag_t;

typedef struct debug_node_ref_t {
    debug_node_tag_t tag;
    void *ptr;
} debug_node_ref_t;

const char *debug_node_name(debug_node_ref_t debug_node);
debug_node_ref_t *debug_node_children(debug_node_ref_t debug_node);

#endif  // DEBUG_NODE_H_
