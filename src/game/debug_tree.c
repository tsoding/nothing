#include <assert.h>
#include <stdlib.h>

#include "debug_tree.h"
#include "system/error.h"
#include "system/lt.h"

struct debug_tree_t
{
    lt_t *lt;
    const sprite_font_t *font;
};

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

    debug_tree->font = font;

    return debug_tree;
}

void destroy_debug_tree(debug_tree_t *debug_tree)
{
    assert(debug_tree);
    RETURN_LT0(debug_tree->lt);
}
