#ifndef RECT_H_
#define RECT_H_

struct rect_t {
    float x, y, w, h;
};

int rects_intersect(const struct rect_t *rect1,
                    const struct rect_t *rect2);

#endif  // RECT_H_
