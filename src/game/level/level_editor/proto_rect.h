#ifndef PROTO_RECT_H_
#define PROTO_RECT_H_

typedef struct Layer Layer;

typedef struct {
    bool active;
    Sint32 x, y;
    Vec begin, end;

    Color *color_current;
    Layer **layer_current;
} ProtoRect;

int proto_rect_render(const ProtoRect *proto_rect,
                      Camera *camera);
int proto_rect_update(ProtoRect *proto_rect,
                      float delta_time);
int proto_rect_event(ProtoRect *proto_rect,
                     const SDL_Event *event,
                     const Camera *camera);
int proto_rect_mouse_button(ProtoRect *proto_rect,
                            const SDL_MouseButtonEvent *event,
                            const Camera *camera);
int proto_rect_mouse_motion(ProtoRect *proto_rect,
                            const SDL_MouseMotionEvent *event,
                            const Camera *camera);

#endif  // PROTO_RECT_H_
