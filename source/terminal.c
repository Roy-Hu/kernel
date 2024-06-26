/* source file for functions in terminal */
#include "terminal.h"
#include <comp421/hardware.h>
#include "pcb.h"
#include <stddef.h>

Terminal my_term[NUM_TERMINALS];
/* array of pointers to read queue for terminals */
PCB *read_queue[NUM_TERMINALS];
/* array of pointers to write queue for terminals */
PCB *write_queue[NUM_TERMINALS];



void init_term() {
    int i = 0;
    for (; i < NUM_TERMINALS; ++i) {
        my_term[i].buf_len = 0;
        write_queue[i] = NULL;
        read_queue[i] = NULL;
        my_term[i].trans_proc = NULL;
    }
}