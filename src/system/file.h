#ifndef FILE_H_
#define FILE_H_

#include <time.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#ifndef _WIN32
#include <dirent.h>
#endif

#include "system/s.h"

#ifdef _WIN32
struct dirent
{
    char d_name[MAX_PATH+1];
};

typedef struct DIR DIR;

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
void closedir(DIR *dirp);
#endif

String read_whole_file(Memory *memory, const char *filepath);

#endif  // FILE_H_
