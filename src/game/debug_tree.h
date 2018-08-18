#ifndef DEBUG_TREE_H_
#define DEBUG_TREE_H_

#include <SDL2/SDL.h>

#include "debug_node.h"

typedef struct Debug_tree Debug_tree;
typedef struct Sprite_font Sprite_font;

Debug_tree *create_debug_tree(const Sprite_font *font);
void destroy_debug_tree(Debug_tree *debug_tree);

int debug_tree_render(const Debug_tree *debug_tree,
                      SDL_Renderer *renderer);

void debug_tree_toggle_enabled(Debug_tree *debug_tree);

#endif  // DEBUG_TREE_H_
