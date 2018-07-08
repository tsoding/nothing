#ifndef ERROR_H_
#define ERROR_H_

typedef enum error_type_t {
    ERROR_TYPE_OK = 0,
    ERROR_TYPE_LIBC,
    ERROR_TYPE_SDL2,
    ERROR_TYPE_SDL2_MIXER,
    ERROR_TYPE_SDL2_TTF,

    ERROR_TYPE_N
} error_type_t;

error_type_t current_error(void);
void throw_error(error_type_t error_type);
void reset_error(void);
void print_current_error_msg(const char *user_prefix);
void print_error_msg(error_type_t error_type, const char *user_prefix);

#endif  // ERROR_H_
