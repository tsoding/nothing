#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "game/camera.h"
#include "game/level/background.h"
#include "ui/slider.h"

typedef struct {
    Background background;
    // TODO(#1123): the volume_slider is not fully synced with the volume of sound_samples
    Slider volume_slider;
    Vec2f volume_slider_scale;
    Vec2f camera_position;
} Settings;

Settings create_settings(void);

void settings_render(const Settings *settings, const Camera *camera);
void settings_event(Settings *settings, Camera *camera, const SDL_Event *event);
void settings_update(Settings *settings, Camera *camera, float dt);

#endif  // SETTINGS_H_
