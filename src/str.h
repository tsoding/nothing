#ifndef STR_H_
#define STR_H_

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x

char *string_duplicate(const char *str,
                       const char *str_end);
char *string_append(char *prefix,
                    const char *suffix);
char *trim_endline(char *s);

#endif  // STR_H_
