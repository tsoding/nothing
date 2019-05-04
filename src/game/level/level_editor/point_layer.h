#ifndef POINT_LAYER_H_
#define POINT_LAYER_H_

#define ID_MAX_SIZE 36

typedef struct PointLayer PointLayer;
typedef struct LineStream LineStream;
typedef struct Camera Camera;

PointLayer *create_point_layer_from_line_stream(LineStream *line_stream);
void destroy_point_rect_layer(PointLayer *point_rect_layer);

int point_layer_render(const PointLayer *point_rect_layer,
                       Camera *camera);
int point_layer_mouse_button(PointLayer *point_rect_layer,
                             const SDL_MouseButtonEvent *event);
int point_layer_event(PointLayer *point_rect_layer,
                      const SDL_Event *event);

size_t point_layer_count(const PointLayer *point_rect_layer);
const Point *point_layer_points(const PointLayer *point_rect_layer);
const Color *point_layer_colors(const PointLayer *point_rect_layer);
const char *point_layer_ids(const PointLayer *point_rect_layer);

#endif  // POINT_LAYER_H_
