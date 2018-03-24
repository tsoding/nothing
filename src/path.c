#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "./lt.h"
#include "./path.h"
#include "./error.h"

char *base_path_folder(const char *subfolder)
{
    lt_t *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    char *base_path = PUSH_LT(lt, SDL_GetBasePath(), SDL_free);
    if (base_path == NULL) {
        base_path = PUSH_LT(lt, SDL_strdup("./"), SDL_free);
    }

    const size_t buffer_size =
        sizeof(char) * (strlen(base_path) + strlen(subfolder) + 1);

    char *buffer = PUSH_LT(lt, malloc(buffer_size), free);
    if (buffer == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    if (snprintf(buffer, buffer_size,
                 "%s%s/", base_path, subfolder) < 0) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    RETURN_LT(lt, RELEASE_LT(lt, buffer));
}
