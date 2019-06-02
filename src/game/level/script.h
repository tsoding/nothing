#ifndef SCRIPT_H_
#define SCRIPT_H_

#include <stdbool.h>
#include "ebisp/expr.h"

typedef struct Script Script;
typedef struct LineStream LineStream;
typedef struct Broadcast Broadcast;

Script *create_script_from_string(Broadcast *broadcast, const char *source);
Script *create_script_from_line_stream(LineStream *line_stream,
                                       Broadcast *broadcast);
void destroy_script(Script *script);

// TODO: script_eval should return EvalResult and it should be handled properly everywhere
int script_eval(Script *script, struct Expr expr);

bool script_has_scope_value(const Script *script, const char *name);

const char *script_source_code(const Script *script);

Gc *script_gc(const Script *script);

#endif  // SCRIPT_H_
