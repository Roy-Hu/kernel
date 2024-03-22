#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <comp421/hardware.h>
#include <stddef.h>

typedef struct term {
    char read_buf[TERMINAL_MAX_LINE];
    char *write_buf;
    int buf_len;
} Terminal;

void init_term();

extern Terminal my_term[NUM_TERMINALS];


#endif