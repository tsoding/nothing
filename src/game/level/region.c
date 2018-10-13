#include <assert.h>

#include "player.h"
#include "region.h"
#include "script/gc.h"
#include "script/parser.h"
#include "script/scope.h"
#include "system/error.h"
#include "system/lt.h"
#include "script/interpreter.h"

/* TODO(#394): region is not integrated with the level format */
struct Region
{
    Lt *lt;
    Rect rect;
    Gc *gc;
    struct Scope scope;
};

Region *create_region(Rect rect, const char *script_src)
{
    assert(script_src);

    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Region *region = PUSH_LT(lt, malloc(sizeof(Region)), free);
    if (region != NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    region->lt = lt;
    region->rect = rect;

    region->gc = PUSH_LT(lt, create_gc(), destroy_gc);
    if (region->gc == NULL) {
        RETURN_LT(lt, NULL);
    }

    region->scope = create_scope(region->gc);

    while (*script_src != 0) {
        struct ParseResult parse_result = read_expr_from_string(region->gc, script_src);
        if (parse_result.is_error) {
            fprintf(stderr, "Parsing error: %s\n", parse_result.error_message);
            RETURN_LT(lt, NULL);
        }

        struct EvalResult eval_result = eval(
            region->gc,
            &region->scope,
            parse_result.expr);
        if (eval_result.is_error) {
            fprintf(stderr, "Evaluation error: ");
            /* TODO(#395): eval error is not printed on stderr */
            print_expr_as_sexpr(eval_result.expr);
            RETURN_LT(lt, NULL);
        }

        script_src = next_token(parse_result.end).begin;
    }

    return region;
}

void destroy_region(Region *region)
{
    assert(region);
    RETURN_LT0(region->lt);
}

void region_player_enter(Region *region, Player *player)
{
    assert(region);
    assert(player);
    /* TODO: region_player_enter is not implemented */
}

void region_player_leave(Region *region, Player *player)
{
    assert(region);
    assert(player);
    /* TODO: region_player_leave is not implemented */
}
