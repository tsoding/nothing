#ifndef COLOR_PICKER_H_
#define COLOR_PICKER_H_

typedef struct {
    ProtoRect *proto_rect;
} ColorPicker;

int color_picker_render(const ColorPicker *color_picker,
                        Camera *camera);
int color_picker_mouse_button(ColorPicker *color_picker,
                              const SDL_MouseButtonEvent *event,
                              bool *selected);

#endif  // COLOR_PICKER_H_
