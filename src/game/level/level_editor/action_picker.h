#ifndef ACTION_PICKER_H_
#define ACTION_PICKER_H_

#include "game/level/action.h"
#include "game/camera.h"

typedef struct {
    Action action;
    Vec2f position;
    Vec2f size;
} ActionPicker;


void action_picker_render(const ActionPicker *action_picker,
                          const Camera *camera);
void action_picker_event(ActionPicker *action_picker,
                         const SDL_Event *event);

#endif  // ACTION_PICKER_H_
