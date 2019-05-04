#ifndef POINT_LAYER_H_
#define POINT_LAYER_H_

typedef struct PointLayer PointLayer;
typedef struct LineStream LineStream;
typedef struct Camera Camera;

PointLayer *create_point_layer(void);
PointLayer *create_point_layer_from_line_stream(LineStream *line_stream);
void destroy_point_layer(PointLayer *point_layer);

int point_layer_render(const PointLayer *point_layer,
                       Camera *camera);
int point_layer_event(PointLayer *point_layer,
                      const SDL_Event *event);



#endif  // POINT_LAYER_H_
