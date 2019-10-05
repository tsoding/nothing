#include "system/stacktrace.h"
#include <string.h>

#include "ebisp/gc.h"
#include "ebisp/interpreter.h"
#include "ebisp/builtins.h"
#include "ebisp/scope.h"
#include "ebisp/parser.h"

#include "std.h"

static struct Expr
lambda(Gc *gc, struct Expr args, struct Expr body, struct Scope *scope)
{
    return atom_as_expr(create_lambda_atom(gc, args, body, scope->expr));
}

static struct EvalResult
quasiquote(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    trace_assert(gc);
    trace_assert(scope);

    struct Expr expr = void_expr();
    struct EvalResult result = match_list(gc, "e", args, &expr);
    if (result.is_error) {
        return result;
    }

    const char *unquote = NULL;
    struct Expr unquote_expr = void_expr();
    result = match_list(gc, "qe", expr, &unquote, &unquote_expr);

    if (!result.is_error && strcmp(unquote, "unquote") == 0) {
        return eval(gc, scope, unquote_expr);
    } else if (cons_p(expr)) {
        struct EvalResult left = quasiquote(param, gc, scope, CONS(gc, CAR(expr), NIL(gc)));
        if (left.is_error) {
            return left;
        }
        struct EvalResult right = quasiquote(param, gc, scope, CONS(gc, CDR(expr), NIL(gc)));
        if (right.is_error) {
            return right;
        }
        return eval_success(CONS(gc, left.expr, right.expr));
    } else {
        return eval_success(expr);
    }
}

static struct EvalResult
unquote(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    trace_assert(gc);
    trace_assert(scope);
    (void) args;

    return eval_failure(STRING(gc, "Using unquote outside of quasiquote."));
}

static struct EvalResult
greaterThan(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    trace_assert(gc);
    trace_assert(scope);
    (void) param;

    // TODO(#1099): greaterThan does not support floats

    long int x1 = 0;
    struct Expr xs = void_expr();

    struct EvalResult result = match_list(gc, "d*", args, &x1, &xs);
    if (result.is_error) {
        return result;
    }

    bool sorted = true;

    while (!nil_p(xs) && sorted) {
        long int x2 = 0;
        result = match_list(gc, "d*", xs, &x2, NULL);
        if (result.is_error) {
            return result;
        }

        sorted = sorted && (x1 > x2);
        args = xs;

        result = match_list(gc, "d*", args, &x1, &xs);
        if (result.is_error) {
            return result;
        }
    }

    return eval_success(bool_as_expr(gc, sorted));
}

static struct EvalResult
list_op(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    trace_assert(gc);
    trace_assert(scope);
    (void) param;

    return eval_success(args);
}

static struct EvalResult
plus_op(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    trace_assert(gc);
    trace_assert(scope);

    // TODO(#1100): plus_op does not support floats

    long int result = 0L;

    while (!nil_p(args)) {
        if (!cons_p(args)) {
            return wrong_argument_type(gc, "consp", args);
        }

        if (!integer_p(CAR(args))) {
            return wrong_argument_type(gc, "integerp", CAR(args));
        }

        result += CAR(args).atom->num;
        args = CDR(args);
    }

    return eval_success(INTEGER(gc, result));
}

static struct EvalResult
mul_op(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    trace_assert(gc);
    trace_assert(scope);

    long int result = 1L;

    // TODO(#1101): mul_op does not support floats

    while (!nil_p(args)) {
        if (!cons_p(args)) {
            return wrong_argument_type(gc, "consp", args);
        }

        if (!integer_p(CAR(args))) {
            return wrong_argument_type(gc, "integerp", CAR(args));
        }

        result *= CAR(args).atom->num;
        args = CDR(args);
    }

    return eval_success(INTEGER(gc, result));
}

static struct EvalResult
assoc_op(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    trace_assert(gc);
    trace_assert(scope);

    struct Expr key = NIL(gc);
    struct Expr alist = NIL(gc);
    struct EvalResult result = match_list(gc, "ee", args, &key, &alist);
    if (result.is_error) {
        return result;
    }

    return eval_success(assoc(key, alist));
}

static struct EvalResult
set(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    trace_assert(gc);
    trace_assert(scope);

    const char *name = NULL;
    struct Expr value = void_expr();
    struct EvalResult result = match_list(gc, "qe", args, &name, &value);
    if (result.is_error) {
        return result;
    }

    result = eval(gc, scope, value);
    if (result.is_error) {
        return result;
    }

    set_scope_value(gc, scope, SYMBOL(gc, name), result.expr);

    return eval_success(result.expr);
}

static struct EvalResult
quote(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    trace_assert(gc);
    trace_assert(scope);

    struct Expr expr = void_expr();
    struct EvalResult result = match_list(gc, "e", args, &expr);
    if (result.is_error) {
        return result;
    }

    return eval_success(expr);
}

static struct EvalResult
begin(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    trace_assert(gc);
    trace_assert(scope);

    struct Expr block = void_expr();
    struct EvalResult result = match_list(gc, "*", args, &block);
    if (result.is_error) {
        return result;
    }

    return eval_block(gc, scope, block);
}

static struct EvalResult
defun(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    trace_assert(gc);
    trace_assert(scope);

    struct Expr name = void_expr();
    struct Expr args_list = void_expr();
    struct Expr body = void_expr();

    struct EvalResult result = match_list(gc, "ee*", args, &name, &args_list, &body);
    if (result.is_error) {
        return result;
    }

    if (!list_of_symbols_p(args_list)) {
        return wrong_argument_type(gc, "list-of-symbolsp", args_list);
    }

    return eval(gc, scope,
                list(gc, "qee", "set", name,
                            lambda(gc, args_list, body, scope)));
}

static struct EvalResult
when(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    trace_assert(gc);
    trace_assert(scope);

    struct Expr condition = void_expr();
    struct Expr body = void_expr();

    struct EvalResult result = match_list(
        gc, "e*", args, &condition, &body);
    if (result.is_error) {
        return result;
    }

    result = eval(gc, scope, condition);
    if (result.is_error) {
        return result;
    }

    if (!nil_p(result.expr)) {
        return eval_block(gc, scope, body);
    }

    return eval_success(NIL(gc));
}

static struct EvalResult
lambda_op(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    trace_assert(gc);
    trace_assert(scope);

    struct Expr args_list = void_expr();
    struct Expr body = void_expr();

    struct EvalResult result = match_list(gc, "e*", args, &args_list, &body);
    if (result.is_error) {
        return result;
    }

    if (!list_of_symbols_p(args_list)) {
        return wrong_argument_type(gc, "list-of-symbolsp", args_list);
    }

    return eval_success(lambda(gc, args_list, body, scope));
}

static struct EvalResult
equal_op(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    trace_assert(gc);
    trace_assert(scope);

    struct Expr obj1;
    struct Expr obj2;
    struct EvalResult result = match_list(gc, "ee", args, &obj1, &obj2);
    if (result.is_error) {
        return result;
    }

    if (equal(obj1, obj2)) {
        return eval_success(T(gc));
    } else {
        return eval_success(NIL(gc));
    }
}

static struct EvalResult
load(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    trace_assert(gc);
    trace_assert(scope);

    const char *filename = NULL;
    struct EvalResult result = match_list(gc, "s", args, &filename);
    if (result.is_error) {
        return result;
    }

    struct ParseResult parse_result = read_all_exprs_from_file(gc, filename);
    if (parse_result.is_error) {
        /* TODO(#599): (load) does not provide position of the parse error  */
        return read_error(gc, parse_result.error_message, 0);
    }

    return eval_block(gc, scope, parse_result.expr);
}

// TODO(#672): append does not work with arbitrary amount of arguments
// TODO(#673): append is implemented recursively
//   It's very StackOverflow prone
static struct EvalResult
append(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    trace_assert(gc);
    trace_assert(scope);

    struct Expr xs = void_expr();
    struct Expr ys = void_expr();
    struct EvalResult result = match_list(gc, "ee", args, &xs, &ys);
    if (result.is_error) {
        return result;
    }

    if (nil_p(xs)) {
        return eval_success(ys);
    }

    struct Expr xs1 = void_expr();
    struct Expr x = void_expr();
    result = match_list(gc, "e*", xs, &x, &xs1);
    if (result.is_error) {
        return result;
    }

    result = append(param, gc, scope, list(gc, "ee", xs1, ys));
    if (result.is_error) {
        return result;
    }

    return eval_success(CONS(gc, x, result.expr));
}

void load_std_library(Gc *gc, struct Scope *scope)
{
    set_scope_value(gc, scope, SYMBOL(gc, "car"), NATIVE(gc, car, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, ">"), NATIVE(gc, greaterThan, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "+"), NATIVE(gc, plus_op, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "*"), NATIVE(gc, mul_op, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "list"), NATIVE(gc, list_op, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "t"), SYMBOL(gc, "t"));
    set_scope_value(gc, scope, SYMBOL(gc, "nil"), SYMBOL(gc, "nil"));
    set_scope_value(gc, scope, SYMBOL(gc, "assoc"), NATIVE(gc, assoc_op, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "quasiquote"), NATIVE(gc, quasiquote, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "set"), NATIVE(gc, set, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "quote"), NATIVE(gc, quote, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "begin"), NATIVE(gc, begin, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "defun"), NATIVE(gc, defun, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "when"), NATIVE(gc, when, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "lambda"), NATIVE(gc, lambda_op, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "λ"), NATIVE(gc, lambda_op, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "unquote"), NATIVE(gc, unquote, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "load"), NATIVE(gc, load, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "append"), NATIVE(gc, append, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "equal"), NATIVE(gc, equal_op, NULL));
}
