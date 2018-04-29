#include <assert.h>

#include "game/level/background.h"
#include "system/lt.h"

struct background_t
{
    lt_t *lt;
    color_t base_color;
};

background_t *create_background(color_t base_color)
{
    lt_t *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    background_t *background = PUSH_LT(lt, malloc(sizeof(background_t)), free);
    if (background == NULL) {
        RETURN_LT(lt, NULL);
    }

    background->base_color = base_color;
    background->lt = lt;

    return background;
}

void destroy_background(background_t *background)
{
    assert(background);
    RETURN_LT0(background->lt);
}

int background_render(const background_t *background,
                      SDL_Renderer *renderer,
                      const camera_t *camera)
{
    assert(background);
    assert(renderer);
    assert(camera);
    return 0;
}
