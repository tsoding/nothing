#ifndef SLIDER_H_
#define SLIDER_H_

typedef struct {
    int drag;
    float value;
    float max_value;
} Slider;

typedef union SDL_Event SDL_Event;

int slider_render(const Slider *slider, const Camera *camera, Rect boundary);
int slider_event(Slider *slider, const SDL_Event *event, Rect boundary, int *selected);

#endif  // SLIDER_H_
