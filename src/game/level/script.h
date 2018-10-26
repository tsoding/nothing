#ifndef SCRIPT_H_
#define SCRIPT_H_

typedef struct Script Script;
typedef struct LineStream LineStream;

Script *create_script_from_line_stream(LineStream *line_stream);
void destroy_script(Script *script);

#endif  // SCRIPT_H_
