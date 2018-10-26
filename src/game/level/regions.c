#include <assert.h>

#include "player.h"
#include "regions.h"
#include "script/gc.h"
#include "script/interpreter.h"
#include "script/parser.h"
#include "script/scope.h"
#include "str.h"
#include "system/error.h"
#include "system/lt.h"
#include "system/line_stream.h"

/* TODO(#394): regions is not integrated with the level format */
struct Regions
{
    Lt *lt;
    Rect rect;
    Gc *gc;
    struct Scope scope;
};

Regions *create_regions(Rect rect, const char *script_src)
{
    assert(script_src);

    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Regions *regions = PUSH_LT(lt, malloc(sizeof(Regions)), free);
    if (regions != NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    regions->lt = lt;
    regions->rect = rect;

    regions->gc = PUSH_LT(lt, create_gc(), destroy_gc);
    if (regions->gc == NULL) {
        RETURN_LT(lt, NULL);
    }

    regions->scope = create_scope(regions->gc);

    while (*script_src != 0) {
        struct ParseResult parse_result = read_expr_from_string(regions->gc, script_src);
        if (parse_result.is_error) {
            fprintf(stderr, "Parsing error: %s\n", parse_result.error_message);
            RETURN_LT(lt, NULL);
        }

        struct EvalResult eval_result = eval(
            regions->gc,
            &regions->scope,
            parse_result.expr);
        if (eval_result.is_error) {
            fprintf(stderr, "Evaluation error: ");
            print_expr_as_sexpr(stderr, eval_result.expr);
            RETURN_LT(lt, NULL);
        }

        script_src = next_token(parse_result.end).begin;
    }

    /* TODO(#405): regions does not check if the script provides on-enter and on-leave callbacks */

    return regions;
}

Regions *create_regions_from_stream(LineStream *line_stream)
{
    assert(line_stream);

    Rect rect;

    if (sscanf(
            line_stream_next(line_stream),
            "%f%f%f%f",
            &rect.x, &rect.y,
            &rect.w, &rect.h) < 0) {
        throw_error(ERROR_TYPE_LIBC);
        return NULL;
    }

    size_t n;
    if (sscanf(
            line_stream_next(line_stream),
            "%lu\n",
            &n) < 0) {
        throw_error(ERROR_TYPE_LIBC);
        return NULL;
    }

    /* TODO: script is not read */
    char *script = NULL;

    Regions *regions = create_regions(rect, script);
    if (regions == NULL) {
        return NULL;
    }

    free(script);

    return regions;
}

void destroy_regions(Regions *regions)
{
    assert(regions);
    RETURN_LT0(regions->lt);
}

void regions_player_enter(Regions *regions, Player *player)
{
    assert(regions);
    assert(player);
    /* TODO(#396): regions_player_enter is not implemented */
}

void regions_player_leave(Regions *regions, Player *player)
{
    assert(regions);
    assert(player);
    /* TODO(#397): regions_player_leave is not implemented */
}
