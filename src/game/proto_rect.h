#ifndef PROTO_RECT_H_
#define PROTO_RECT_H_

typedef struct ProtoRect ProtoRect;

ProtoRect *create_proto_rect(void);
void destroy_proto_rect(ProtoRect *proto_rect);

int proto_rect_render(const ProtoRect *proto_rect,
                       Camera *camera);
int proto_rect_update(ProtoRect *proto_rect,
                       float delta_time);
int proto_rect_events(ProtoRect *proto_rect,
                       SDL_Event *event);

#endif  // PROTO_RECT_H_
