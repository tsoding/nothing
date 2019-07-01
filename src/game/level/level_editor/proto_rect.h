#ifndef PROTO_RECT_H_
#define PROTO_RECT_H_

typedef struct RectLayer RectLayer;

typedef struct {
    bool active;
    Sint32 x, y;
    Vec begin, end;
} ProtoRect;

int proto_rect_render(const ProtoRect *proto_rect,
                      Camera *camera,
                      Color color);
int proto_rect_event(ProtoRect *proto_rect,
                     const SDL_Event *event,
                     const Camera *camera,
                     Color color,
                     RectLayer *layer);

#endif  // PROTO_RECT_H_
