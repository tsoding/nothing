#ifndef POINT_LAYER_H_
#define POINT_LAYER_H_

#include "math/point.h"
#include "color.h"
#include "layer.h"

#define ID_MAX_SIZE 36

typedef struct PointLayer PointLayer;
typedef struct LineStream LineStream;
typedef struct Camera Camera;

LayerPtr point_layer_as_layer(PointLayer *point_layer);
PointLayer *create_point_layer_from_line_stream(LineStream *line_stream);
void destroy_point_layer(PointLayer *point_layer);

int point_layer_render(const PointLayer *point_layer,
                       Camera *camera,
                       float fa);
int point_layer_event(PointLayer *point_layer,
                      const SDL_Event *event,
                      const Camera *camera);

int point_layer_dump_stream(const PointLayer *point_layer,
                            FILE *filedump);

size_t point_layer_count(const PointLayer *point_layer);
const Point *point_layer_points(const PointLayer *point_layer);
const Color *point_layer_colors(const PointLayer *point_layer);
const char *point_layer_ids(const PointLayer *point_layer);

#endif  // POINT_LAYER_H_
