#ifndef LIST_SELECTOR_H_
#define LIST_SELECTOR_H_

#include "game/camera.h"

typedef struct ListSelector ListSelector;

ListSelector *create_list_selector(const char *items[],
                                   size_t count,
                                   Vec2f font_scale,
                                   float padding_bottom);
void destroy_list_selector(ListSelector *list_selector);

int list_selector_render(const Camera *camera,
                         const ListSelector *list_selector);
Vec2f list_selector_size(const ListSelector *list_selector,
                         Vec2f font_scale,
                         float padding_bottom);

int list_selector_update(ListSelector *list_selector, float delta_time);
int list_selector_event(ListSelector *list_selector, const SDL_Event *event);

int list_selector_selected(const ListSelector *list_selector);
void list_selector_clean_selection(ListSelector *list_selector);

void list_selector_move(ListSelector *list_selector, Vec2f position);

#endif  // LIST_SELECTOR_H_
