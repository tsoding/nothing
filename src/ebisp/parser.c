#include "system/stacktrace.h"
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "ebisp/builtins.h"
#include "ebisp/parser.h"
#include "system/lt.h"
#include "system/lt_adapters.h"

#define MAX_BUFFER_LENGTH (5 * 1000 * 1000)

static struct ParseResult parse_expr(Gc *gc, struct Token current_token);

static struct ParseResult parse_cdr(Gc *gc, struct Token current_token)
{
    if (*current_token.begin != '.') {
        return parse_failure("Expected .", current_token.begin);
    }

    struct ParseResult cdr = read_expr_from_string(gc, current_token.end);
    if (cdr.is_error) {
        return cdr;
    }

    current_token = next_token(cdr.end);

    if (*current_token.begin != ')') {
        return parse_failure("Expected )", current_token.begin);
    }

    return parse_success(cdr.expr, current_token.end);
}

static struct ParseResult parse_list_end(Gc *gc, struct Token current_token)
{
    if (*current_token.begin != ')') {
        return parse_failure("Expected )", current_token.begin);
    }

    return parse_success(atom_as_expr(create_symbol_atom(gc, "nil", NULL)),
                         current_token.end);
}

static struct ParseResult parse_list(Gc *gc, struct Token current_token)
{
    if (*current_token.begin != '(') {
        return parse_failure("Expected (", current_token.begin);
    }

    current_token = next_token(current_token.end);

    if (*current_token.begin == ')') {
        return parse_list_end(gc, current_token);
    }

    struct ParseResult car = parse_expr(gc, current_token);
    if (car.is_error) {
        return car;
    }

    struct Cons *list = create_cons(gc, car.expr, void_expr());
    struct Cons *cons = list;
    current_token = next_token(car.end);

    while (*current_token.begin != '.' &&
           *current_token.begin != ')' &&
           *current_token.begin != 0) {
        car = parse_expr(gc, current_token);
        if (car.is_error) {
            return car;
        }

        cons->cdr = cons_as_expr(create_cons(gc, car.expr, void_expr()));
        cons = cons->cdr.cons;

        current_token = next_token(car.end);
    }

    struct ParseResult cdr = *current_token.begin == '.'
        ? parse_cdr(gc, current_token)
        : parse_list_end(gc, current_token);

    if (cdr.is_error) {
        return cdr;
    }

    cons->cdr = cdr.expr;

    return parse_success(cons_as_expr(list), cdr.end);
}

static struct ParseResult parse_string(Gc *gc, struct Token current_token)
{
    if (*current_token.begin != '"') {
        return parse_failure("Expected \"", current_token.begin);
    }

    if (*(current_token.end - 1) != '"') {
        return parse_failure("Unclosed string", current_token.begin);
    }

    if (current_token.begin + 1 == current_token.end) {
        return parse_success(atom_as_expr(create_string_atom(gc, "", NULL)),
                             current_token.end);
    }

    return parse_success(
        atom_as_expr(
            create_string_atom(gc, current_token.begin + 1, current_token.end - 1)),
        current_token.end);
}

static struct ParseResult parse_integer(Gc *gc, struct Token current_token)
{
    char *endptr = 0;
    const long int x = strtol(current_token.begin, &endptr, 10);

    if ((current_token.begin == endptr) || (current_token.end != endptr)) {
        return parse_failure("Expected integer", current_token.begin);
    }

    return parse_success(
        atom_as_expr(create_integer_atom(gc, x)),
        current_token.end);
}

static struct ParseResult parse_real(Gc *gc, struct Token current_token)
{
    trace_assert(gc);

    char *endptr = 0;
    const float x = strtof(current_token.begin, &endptr);

    if ((current_token.begin == endptr) || (current_token.end != endptr)) {
        return parse_failure("Expected real", current_token.begin);
    }

    return parse_success(REAL(gc, x), current_token.end);
}

static struct ParseResult parse_symbol(Gc *gc, struct Token current_token)
{
    if (*current_token.begin == 0) {
        return parse_failure("EOF", current_token.begin);
    }

    return parse_success(
        atom_as_expr(create_symbol_atom(gc, current_token.begin, current_token.end)),
        current_token.end);
}

static struct ParseResult parse_expr(Gc *gc, struct Token current_token)
{
    if (*current_token.begin == 0) {
        return parse_failure("EOF", current_token.begin);
    }

    switch (*current_token.begin) {
    case '(': return parse_list(gc, current_token);
    /* TODO(#292): parser does not support escaped string characters */
    case '"': return parse_string(gc, current_token);
    case '\'': {
        struct ParseResult result = parse_expr(gc, next_token(current_token.end));

        if (result.is_error) {
            return result;
        }

        result.expr = list(gc, "qe", "quote", result.expr);

        return result;
    } break;

    case '`': {
        struct ParseResult result = parse_expr(gc, next_token(current_token.end));

        if (result.is_error) {
            return result;
        }

        result.expr = list(gc, "qe", "quasiquote", result.expr);

        return result;
    } break;

    case ',': {
        struct ParseResult result = parse_expr(gc, next_token(current_token.end));

        if (result.is_error) {
            return result;
        }

        result.expr = list(gc, "qe", "unquote", result.expr);

        return result;
    } break;

    default: {}
    }

    if (*current_token.begin == '-' || isdigit(*current_token.begin)) {
        struct ParseResult result;

        result = parse_integer(gc, current_token); if (!result.is_error) return result;
        result = parse_real(gc, current_token); if (!result.is_error) return result;
    }

    return parse_symbol(gc, current_token);
}

struct ParseResult read_expr_from_string(Gc *gc, const char *str)
{
    trace_assert(gc);
    trace_assert(str);
    return parse_expr(gc, next_token(str));
}

struct ParseResult read_all_exprs_from_string(Gc *gc, const char *str)
{
    trace_assert(gc);
    trace_assert(str);

    struct Token current_token = next_token(str);
    if (*current_token.end == 0) {
        return parse_success(NIL(gc), current_token.end);
    }

    struct ParseResult parse_result = parse_expr(gc, current_token);
    if (parse_result.is_error) {
        return parse_result;
    }

    struct Cons *head = create_cons(gc, parse_result.expr, void_expr());
    struct Cons *cons = head;

    current_token = next_token(parse_result.end);
    while (*current_token.end != 0) {
        parse_result = parse_expr(gc, current_token);
        if (parse_result.is_error) {
            return parse_result;
        }

        cons->cdr = CONS(gc, parse_result.expr, void_expr());
        cons = cons->cdr.cons;
        current_token = next_token(parse_result.end);
    }

    cons->cdr = NIL(gc);

    return parse_success(cons_as_expr(head), parse_result.end);
}

struct ParseResult read_expr_from_file(Gc *gc, const char *filename)
{
    trace_assert(filename);

    Lt *lt = create_lt();

    FILE *stream = PUSH_LT(lt, fopen(filename, "rb"), fclose_lt);
    if (!stream) {
        /* TODO(#307): ParseResult should not be used for reporting IO failures */
        RETURN_LT(lt, parse_failure(strerror(errno), NULL));
    }

    if (fseek(stream, 0, SEEK_END) != 0) {
        RETURN_LT(lt, parse_failure("Could not find the end of the file", NULL));
    }

    const long int buffer_length = ftell(stream);

    if (buffer_length < 0) {
        RETURN_LT(lt, parse_failure("Couldn't get the size of file", NULL));
    }

    if (buffer_length == 0) {
        RETURN_LT(lt, parse_failure("File is empty", NULL));
    }

    if (buffer_length >= MAX_BUFFER_LENGTH) {
        RETURN_LT(lt, parse_failure("File is too big", NULL));
    }

    if (fseek(stream, 0, SEEK_SET) != 0) {
        RETURN_LT(lt, parse_failure("Could not find the beginning of the file", NULL));
    }

    char * const buffer = PUSH_LT(lt, malloc((size_t) buffer_length + 1), free);
    if (buffer == NULL) {
        RETURN_LT(lt, parse_failure(strerror(errno), NULL));
    }

    if (fread(buffer, 1, (size_t) buffer_length, stream) != (size_t) buffer_length) {
        RETURN_LT(lt, parse_failure("Could not read the file", NULL));
    }

    struct ParseResult result = read_expr_from_string(gc, buffer);

    RETURN_LT(lt, result);
}

/* TODO(#598): duplicate code in read_all_exprs_from_file and read_expr_from_file  */
struct ParseResult read_all_exprs_from_file(Gc *gc, const char *filename)
{
    trace_assert(filename);

    Lt *lt = create_lt();

    FILE *stream = PUSH_LT(lt, fopen(filename, "rb"), fclose_lt);
    if (!stream) {
        RETURN_LT(lt, parse_failure(strerror(errno), NULL));
    }

    if (fseek(stream, 0, SEEK_END) != 0) {
        RETURN_LT(lt, parse_failure("Could not find the end of the file", NULL));
    }

    const long int buffer_length = ftell(stream);

    if (buffer_length < 0) {
        RETURN_LT(lt, parse_failure("Couldn't get the size of file", NULL));
    }

    if (buffer_length == 0) {
        RETURN_LT(lt, parse_failure("File is empty", NULL));
    }

    if (buffer_length >= MAX_BUFFER_LENGTH) {
        RETURN_LT(lt, parse_failure("File is too big", NULL));
    }

    if (fseek(stream, 0, SEEK_SET) != 0) {
        RETURN_LT(lt, parse_failure("Could not find the beginning of the file", NULL));
    }

    char * const buffer = PUSH_LT(lt, malloc((size_t) buffer_length + 1), free);
    if (buffer == NULL) {
        RETURN_LT(lt, parse_failure(strerror(errno), NULL));
    }

    if (fread(buffer, 1, (size_t) buffer_length, stream) != (size_t) buffer_length) {
        RETURN_LT(lt, parse_failure("Could not read the file", NULL));
    }

    struct ParseResult result = read_all_exprs_from_string(gc, buffer);

    RETURN_LT(lt, result);
}

struct ParseResult parse_success(struct Expr expr,
                                 const char *end)
{
    struct ParseResult result = {
        .is_error = false,
        .expr = expr,
        .end = end
    };

    return result;
}

struct ParseResult parse_failure(const char *error_message,
                                 const char *end)
{
    struct ParseResult result = {
        .is_error = true,
        .error_message = error_message,
        .end = end
    };

    return result;
}

void print_parse_error(FILE *stream,
                       const char *str,
                       struct ParseResult result)
{
    /* TODO(#294): print_parse_error doesn't support multiple lines */
    if (!result.is_error) {
        return;
    }

    if (result.end) {
        fprintf(stream, "%s\n", str);
        for (size_t i = 0; i < (size_t) (result.end - str); ++i) {
            fprintf(stream, " ");
        }
        fprintf(stream, "^\n");
    }

    fprintf(stream, "%s\n", result.error_message);
}
