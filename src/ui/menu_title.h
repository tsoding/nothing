#ifndef MENU_TITLE_H_
#define MENU_TITLE_H_

typedef struct MenuTitle MenuTitle;
typedef struct Camera Camera;

MenuTitle *create_menu_title(const char *text,
                             Vec font_scale);
void destroy_menu_title(MenuTitle *menu_title);

int menu_title_render(const MenuTitle *menu_title,
                      Camera *camera);
int menu_title_update(MenuTitle *menu_title, float delta_time);

Vec menu_title_size(const MenuTitle *menu_title, const Camera *camera);

void menu_title_move(MenuTitle *menu_title,
                     Vec position);

#endif  // MENU_TITLE_H_
