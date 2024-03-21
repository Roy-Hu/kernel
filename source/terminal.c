/* source file for functions in terminal*/
#include "terminal.h"
#include <comp421/hardware.h>
#include <stddef.h>

Terminal my_term[NUM_TERMINALS];

void init_term() {
    int i = 0;
    for (; i < NUM_TERMINALS; ++i) {
        my_term[i].read_buf = NULL;
        my_term[i].write_buf = NULL;
        my_term[i].buf_len = 0;
    }
}