#ifndef FILE_H_
#define FILE_H_

#include <time.h>
#ifndef _WIN32
#include <dirent.h>
#endif

int last_modified(const char *filepath, time_t *time);

#ifdef _WIN32
struct dirent
{
    char d_name[256];
};

typedef struct DIR DIR;

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
void closedir(DIR *dirp);
#endif

#endif  // FILE_H_
