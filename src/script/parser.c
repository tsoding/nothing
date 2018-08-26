#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

#include "script/parser.h"

static struct ParseResult parse_cdr(struct Token current_token)
{
    if (*current_token.begin != '.') {
        return parse_failure("Expected .", current_token.begin);
    }

    struct ParseResult cdr = parse_expr(next_token(current_token.end));
    if (cdr.is_error) {
        return cdr;
    }

    current_token = next_token(cdr.end);

    if (*current_token.begin != ')') {
        destroy_expr(cdr.expr);
        return parse_failure("Expected )", current_token.begin);
    }

    return parse_success(cdr.expr, current_token.end);
}

static struct ParseResult parse_list_end(struct Token current_token)
{
    if (*current_token.begin != ')') {
        return parse_failure("Expected )", current_token.begin);
    }

    return parse_success(atom_as_expr(create_symbol_atom("nil", NULL)),
                         current_token.end);
}

static struct ParseResult parse_list(struct Token current_token)
{
    if (*current_token.begin != '(') {
        return parse_failure("Expected (", current_token.begin);
    }

    current_token = next_token(current_token.end);

    if (*current_token.begin == ')') {
        return parse_list_end(current_token);
    }

    struct ParseResult car = parse_expr(current_token);
    if (car.is_error) {
        return car;
    }

    struct Cons *list = create_cons(car.expr, void_expr());
    struct Cons *cons = list;
    current_token = next_token(car.end);

    while (*current_token.begin != '.' &&
           *current_token.begin != ')' &&
           *current_token.begin != 0) {
        car = parse_expr(current_token);
        if (car.is_error) {
            destroy_cons(list);
            return car;
        }

        cons->cdr = cons_as_expr(create_cons(car.expr, void_expr()));
        cons = cons->cdr.cons;

        current_token = next_token(car.end);
    }

    struct ParseResult cdr = *current_token.begin == '.'
        ? parse_cdr(current_token)
        : parse_list_end(current_token);

    if (cdr.is_error) {
        destroy_cons(list);
        return cdr;
    }

    cons->cdr = cdr.expr;

    return parse_success(cons_as_expr(list), cdr.end);
}

static struct ParseResult parse_string(struct Token current_token)
{
    if (*current_token.begin != '"') {
        return parse_failure("Expected \"", current_token.begin);
    }

    if (*(current_token.end - 1) != '"') {
        return parse_failure("Unclosed string", current_token.begin);
    }

    if (current_token.begin + 1 == current_token.end) {
        return parse_success(atom_as_expr(create_string_atom("", NULL)),
                             current_token.end);
    }

    return parse_success(
        atom_as_expr(
            create_string_atom(current_token.begin + 1, current_token.end - 1)),
        current_token.end);
}

static struct ParseResult parse_number(struct Token current_token)
{
    char *endptr = 0;
    const float x = strtof(current_token.begin, &endptr);

    if (current_token.begin == endptr || current_token.end != endptr) {
        return parse_failure("Expected number", current_token.begin);
    }

    return parse_success(
        atom_as_expr(create_number_atom(x)),
        current_token.end);
}

static struct ParseResult parse_symbol(struct Token current_token)
{
    if (*current_token.begin == 0) {
        return parse_failure("EOF", current_token.begin);
    }

    return parse_success(
        atom_as_expr(create_symbol_atom(current_token.begin, current_token.end)),
        current_token.end);
}

struct ParseResult parse_expr(struct Token current_token)
{
    if (*current_token.begin == 0) {
        return parse_failure("EOF", current_token.begin);
    }

    switch (*current_token.begin) {
    case '(': return parse_list(current_token);
    /* TODO(#292): parser does not support escaped string characters */
    case '"': return parse_string(current_token);
    default: {}
    }

    if (isdigit(*current_token.begin)) {
        return parse_number(current_token);
    }

    return parse_symbol(current_token);
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
    /* TODO(#293): print_parse_error doesn't support colors */
    /* TODO(#294): print_parse_error doesn't support multiple lines */
    if (!result.is_error) {
        return;
    }

    fprintf(stream, "%s\n", str);
    for (size_t i = 0; i < (size_t) (result.end - str); ++i) {
        fprintf(stream, " ");
    }
    fprintf(stream, "^\n");
    fprintf(stream, "%s\n", result.error_message);
}
