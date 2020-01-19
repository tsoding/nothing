#ifndef TEXTURE_H_
#define TEXTURE_H_

SDL_Texture *texture_from_bmp(const char *bmp_file_name,
                              SDL_Renderer *renderer);

#endif  // TEXTURE_H_
