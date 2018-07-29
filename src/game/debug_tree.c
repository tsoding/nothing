#include <assert.h>
#include <stdlib.h>

#include "debug_tree.h"
#include "game/sprite_font.h"
#include "system/error.h"
#include "system/lt.h"

struct debug_tree_t
{
    lt_t *lt;
    const sprite_font_t *font;
    int enabled;
};

/* TODO: debug_tree doesn't render debug_node-s */
debug_tree_t * create_debug_tree(const sprite_font_t *font)
{
    lt_t * const lt = create_lt();

    if (lt == NULL) {
        return NULL;
    }

    debug_tree_t * const debug_tree = PUSH_LT(lt, malloc(sizeof(debug_tree_t)), free);
    if (debug_tree == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    debug_tree->lt = lt;

    debug_tree->enabled = 0;
    debug_tree->font = font;

    return debug_tree;
}

void destroy_debug_tree(debug_tree_t *debug_tree)
{
    assert(debug_tree);
    RETURN_LT0(debug_tree->lt);
}

int debug_tree_render(const debug_tree_t *debug_tree,
                      SDL_Renderer *renderer)
{
    assert(debug_tree);
    assert(renderer);

    if (!debug_tree->enabled) {
        return 0;
    }

    if (sprite_font_render_text(
            debug_tree->font,
            renderer,
            vec(10.0f, 10.0f),
            vec(5.0f, 5.0f),
            color(0.0f, 0.0f, 0.0f, 1.0f),
            "Hello Debug Tree") < 0) {
        return -1;
    }

    return 0;
}

void debug_tree_toggle_enabled(debug_tree_t *debug_tree)
{
    debug_tree->enabled = !debug_tree->enabled;
}
