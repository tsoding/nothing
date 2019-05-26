#ifndef SCRIPT_H_
#define SCRIPT_H_

#include <stdbool.h>

typedef struct Script Script;
typedef struct LineStream LineStream;
typedef struct Broadcast Broadcast;

Script *create_script_from_string(Broadcast *broadcast, const char *source);
Script *create_script_from_line_stream(LineStream *line_stream,
                                       Broadcast *broadcast);
void destroy_script(Script *script);

// TODO(#470): script_eval accepting string instead of expr is very error prone
int script_eval(Script *script, const char *source_code);

bool script_has_scope_value(const Script *script, const char *name);

const char *script_source_code(const Script *script);

#endif  // SCRIPT_H_
