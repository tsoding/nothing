#ifndef GRID_H_
#define GRID_H_

#include <assert.h>

#include "system/nth_alloc.h"
#include "math/rect.h"

typedef struct {
    Rect boundary;
} Widget;

typedef struct {
    size_t rows;
    size_t columns;
    Widget *cells[];
} Grid;

static inline
void grid_put_widget(Grid *grid, Widget *widget,
                     size_t row, size_t column)
{
    assert(grid);
    assert(widget);
    assert(row < grid->rows);
    assert(column < grid->columns);
    grid->cells[row * grid->columns + column] = widget;
}

void grid_relayout(Grid *grid, Rect boundary);

#endif  // GRID_H_
