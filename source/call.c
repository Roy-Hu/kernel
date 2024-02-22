#include "call.h";

int Fork(void) {
    return 0;
}

int Exec(char *filename, char **argvec) {
    return 0;
}

void Exit(int status) {
}

int Wait(int *status_ptr) {
    return 0;
}

int GetPid(void) {
    return 0;
}

int Brk(void *addr) {
    return 0;
}

int Delay (int clock_ticks) {
    return 0;
}

int TtyRead(int tty_id, void *buf, int len) {
    return 0;
}

int TtyWrite(int tty_id, void *buf, int len) {
    return 0;
}