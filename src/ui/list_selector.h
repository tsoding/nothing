#ifndef LIST_SELECTOR_H_
#define LIST_SELECTOR_H_

#include "game/camera.h"
#include "dynarray.h"

typedef const char *(*ListItemText)(void *element);

typedef struct {
    Dynarray items;
    size_t cursor;
    int selected_item;
    Vec2f position;
    Vec2f font_scale;
    float padding_bottom;
    ListItemText list_item_text;
} ListSelector;

int list_selector_render(const Camera *camera,
                         const ListSelector *list_selector);
Vec2f list_selector_size(const ListSelector *list_selector,
                         Vec2f font_scale,
                         float padding_bottom);
int list_selector_event(ListSelector *list_selector, const SDL_Event *event);

static inline
void list_selector_clean_selection(ListSelector *list_selector)
{
    trace_assert(list_selector);
    list_selector->selected_item = -1;
}

#endif  // LIST_SELECTOR_H_
