#ifndef SLIDER_H_
#define SLIDER_H_

typedef struct {
    float value;
    float max_value;
} Slider;

typedef struct Camera Camera;
typedef union SDL_Event SDL_Event;

int slider_render(const Slider *slider, Camera *camera, Rect boundary);
int slider_event(Slider *slider, const SDL_Event *event, Rect boundary);

#endif  // SLIDER_H_
