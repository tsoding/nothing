#ifndef MENU_TITLE_H_
#define MENU_TITLE_H_

typedef struct MenuTitle MenuTitle;
typedef struct Camera Camera;

struct MenuTitle
{
    const char *text;
    Vec font_scale;
    float angle;
    Vec position;
};

int menu_title_render(const MenuTitle *menu_title,
                      Camera *camera);
int menu_title_update(MenuTitle *menu_title, float delta_time);
Vec menu_title_size(const MenuTitle *menu_title, const Camera *camera);

#endif  // MENU_TITLE_H_
