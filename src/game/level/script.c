#include "system/stacktrace.h"

#include "ebisp/gc.h"
#include "ebisp/interpreter.h"
#include "ebisp/parser.h"
#include "ebisp/scope.h"
#include "ebisp/std.h"
#include "game/level.h"
#include "game/level_script.h"
#include "script.h"
#include "str.h"
#include "system/line_stream.h"
#include "system/log.h"
#include "system/log_script.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "ui/console.h"

#if (defined(_MSC_VER) && defined(_M_X64)) || \
    (defined(__GNUC__) && defined(__x86_64__))
#define X64_BUILD
#endif

struct Script
{
    Lt *lt;
    Gc *gc;
    struct Scope scope;
};

Script *create_script_from_line_stream(LineStream *line_stream, Level *level)
{
    trace_assert(line_stream);

    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Script *script = PUSH_LT(lt, nth_alloc(sizeof(Script)), free);
    if (script == NULL) {
        RETURN_LT(lt, NULL);
    }
    script->lt = lt;

    script->gc = PUSH_LT(lt, create_gc(), destroy_gc);
    if (script->gc == NULL) {
        RETURN_LT(lt, NULL);
    }

    script->scope = create_scope(script->gc);

    load_std_library(script->gc, &script->scope);
    load_log_library(script->gc, &script->scope);
    load_level_library(script->gc, &script->scope, level);

    size_t n = 0;
    sscanf(line_stream_next(line_stream), 
#ifdef X64_BUILD
            "%llu",
#else
            "%lu",
#endif
           &n);

    char *source_code = NULL;
    for (size_t i = 0; i < n; ++i) {
        /* TODO(#466): maybe source_code should be constantly replaced in the Lt */
        source_code = string_append(
            source_code,
            line_stream_next(line_stream));
    }
    PUSH_LT(lt, source_code, free);

    struct ParseResult parse_result =
        read_all_exprs_from_string(
            script->gc,
            source_code);
    if (parse_result.is_error) {
        log_fail("Parsing error: %s\n", parse_result.error_message);
        RETURN_LT(lt, NULL);
    }

    struct EvalResult eval_result = eval(
        script->gc,
        &script->scope,
        CONS(script->gc,
             SYMBOL(script->gc, "begin"),
             parse_result.expr));
    if (eval_result.is_error) {
        print_expr_as_sexpr(stderr, eval_result.expr);
        log_fail("\n");
        RETURN_LT(lt, NULL);
    }

    gc_collect(script->gc, script->scope.expr);

    free(RELEASE_LT(lt, source_code));

    return script;
}

void destroy_script(Script *script)
{
    trace_assert(script);
    RETURN_LT0(script->lt);
}

int script_eval(Script *script, const char *source_code)
{
    trace_assert(script);
    trace_assert(source_code);

    struct ParseResult parse_result = read_expr_from_string(
        script->gc,
        source_code);
    if (parse_result.is_error) {
        log_fail("Parsing error: %s\n", parse_result.error_message);
        return -1;
    }

    struct EvalResult eval_result = eval(
        script->gc,
        &script->scope,
        parse_result.expr);
    if (eval_result.is_error) {
        log_fail("Evaluation error: ");
        /* TODO(#521): Evalation error is prepended with `[FAIL]` at the end of the message */
        /* TODO(#486): print_expr_as_sexpr could not be easily integrated with log_fail */
        print_expr_as_sexpr(stderr, eval_result.expr);
        log_fail("\n");
        return -1;
    }

    gc_collect(script->gc, script->scope.expr);

    return 0;
}

bool script_has_scope_value(const Script *script, const char *name)
{
    return !nil_p(
        get_scope_value(
            &script->scope,
            SYMBOL(script->gc, name)));
}
