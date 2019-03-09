#ifndef LIST_SELECTOR_H_
#define LIST_SELECTOR_H_

typedef struct ListSelector ListSelector;

ListSelector *create_list_selector(const Sprite_font *sprite_font,
                                   const char *items[],
                                   size_t count);
void destroy_list_selector(ListSelector *list_selector);

int list_selector_render(const ListSelector *list_selector,
                         SDL_Renderer *renderer,
                         Vec position,
                         float width);
int list_selector_update(ListSelector *list_selector, float delta_time);
int list_selector_event(ListSelector *list_selector, const SDL_Event *event);

const char *list_selector_selected(const ListSelector *list_selector);

#endif  // LIST_SELECTOR_H_
