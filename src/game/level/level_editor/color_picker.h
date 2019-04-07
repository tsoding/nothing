#ifndef COLOR_PICKER_H_
#define COLOR_PICKER_H_

typedef struct {
    ProtoRect *proto_rect;
    Vec position;
} ColorPicker;

int color_picker_render(const ColorPicker *color_picker,
                        Camera *camera);
int color_picker_update(ColorPicker *color_picker,
                        float delta_time);
int color_picker_event(ColorPicker *color_picker,
                       const SDL_Event *event);

#endif  // COLOR_PICKER_H_
