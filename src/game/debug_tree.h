#ifndef DEBUG_TREE_H_
#define DEBUG_TREE_H_

#include <SDL2/SDL.h>

#include "debug_node.h"

typedef struct debug_tree_t debug_tree_t;
typedef struct sprite_font_t sprite_font_t;

debug_tree_t *create_debug_tree(const sprite_font_t *font);
void destroy_debug_tree(debug_tree_t *debug_tree);

int debug_tree_render(const debug_tree_t *debug_tree,
                      SDL_Renderer *renderer);

void debug_tree_toggle_enabled(debug_tree_t *debug_tree);

#endif  // DEBUG_TREE_H_
