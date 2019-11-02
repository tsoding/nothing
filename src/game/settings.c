#include "system/stacktrace.h"
#include "settings.h"

void settings_render(const Settings *settings, const Camera *camera)
{
    trace_assert(settings);
    trace_assert(camera);
}

void settings_event(Settings *settings, const SDL_Event *event)
{
    trace_assert(settings);
    trace_assert(event);
}

void settings_update(Settings *settings, float dt)
{
    trace_assert(settings);
    (void) dt;
}
