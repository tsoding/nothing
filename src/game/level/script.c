#include <assert.h>

#include "script.h"
#include "system/line_stream.h"

/* TODO: script entity clashes with script/ subpackage */
/* TODO: script entity is not implemented */

Script *create_script_from_line_stream(LineStream *line_stream)
{
    assert(line_stream);
    return NULL;
}

void destroy_script(Script *script)
{
    assert(script);
}
