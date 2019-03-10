#ifndef MENU_TITLE_H_
#define MENU_TITLE_H_

typedef struct MenuTitle MenuTitle;

MenuTitle *create_menu_title(const char *text);
void destroy_menu_title(MenuTitle *menu_title);

int menu_title_render(const MenuTitle *menu_title,
                      SDL_Renderer *renderer,
                      Vec position);
int menu_title_update(MenuTitle *menu_title, float delta_time);

Vec menu_title_size(const MenuTitle *menu_title);

#endif  // MENU_TITLE_H_
