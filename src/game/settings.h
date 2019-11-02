#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "game/camera.h"
#include "ui/slider.h"

typedef struct {
    Slider volume_slider;
    Vec2f volume_slider_scale;
} Settings;

void settings_render(const Settings *settings, const Camera *camera);
void settings_event(Settings *settings, Camera *camera, const SDL_Event *event);
void settings_update(Settings *settings, float dt);

#endif  // SETTINGS_H_
