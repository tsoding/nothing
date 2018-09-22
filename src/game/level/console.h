#ifndef CONSOLE_H_
#define CONSOLE_H_

typedef struct Console Console;
typedef struct Level Level;

Console *create_console(Level *level);
void destroy_console(Console *console);

#endif  // CONSOLE_H_
