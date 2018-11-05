#ifndef ERROR_H_
#define ERROR_H_

// TODO(#475): system/error is useless and should be replaced with logging wrappers (similar to system/nth_alloc)

typedef enum Error_type {
    ERROR_TYPE_OK = 0,
    ERROR_TYPE_LIBC,
    ERROR_TYPE_SDL2,
    ERROR_TYPE_SDL2_MIXER,

    ERROR_TYPE_N
} Error_type;

Error_type current_error(void);
void throw_error(Error_type error_type);
void reset_error(void);
void print_current_error_msg(const char *user_prefix);
void print_error_msg(Error_type error_type, const char *user_prefix);

#endif  // ERROR_H_
