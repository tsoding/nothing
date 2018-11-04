#ifndef SCRIPT_H_
#define SCRIPT_H_

typedef struct Script Script;
typedef struct LineStream LineStream;
typedef struct Level Level;

Script *create_script_from_line_stream(LineStream *line_stream,
                                       Level *level);
void destroy_script(Script *script);

// TODO: script_eval accepting string instead of expr is very error prone
int script_eval(Script *script, const char *source_code);

#endif  // SCRIPT_H_
