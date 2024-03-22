#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <comp421/hardware.h>
#include <stddef.h>
#include "pcb.h"

typedef struct term {
    char read_buf[TERMINAL_MAX_LINE];
    int buf_len;
    /* pointing to the current transmitting process */
    PCB* trans_proc;
} Terminal;

void init_term();

extern Terminal my_term[NUM_TERMINALS];


#endif