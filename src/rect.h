#ifndef RECT_H_
#define RECT_H_

typedef struct {
    float x, y, w, h;
} rect_t;

int rects_intersect(const rect_t *rect1,
                    const rect_t *rect2);

#endif  // RECT_H_
