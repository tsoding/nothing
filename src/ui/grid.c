#include "grid.h"

void grid_relayout(Grid grid, Rect boundary)
{
    const float cell_width = boundary.w / (float) grid.columns;
    const float cell_height = boundary.h / (float) grid.rows;

    for (size_t row = 0; row < grid.rows; ++row) {
        for (size_t column = 0; column < grid.columns; ++column) {
            Widget *cell = grid.cells[row * grid.columns + column];
            if (cell) {
                cell->boundary = rect(
                    boundary.x + (float) column * cell_width,
                    boundary.y + (float) row * cell_height,
                    cell_width, cell_height);
            }
        }
    }
}
