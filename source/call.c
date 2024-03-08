#include <stddef.h>

#include <comp421/yalnix.h>

#include "pcb.h"
#include "global.h"

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
    return runningPCB->pid;
}

int Brk(void *addr) {
    return 0;
}

int Delay (int clock_ticks) {
    runningPCB->readyTime = clocktime + clock_ticks;
    runningPCB->state = WAITING;

    TracePrintf(LOG, "Delay %d for pid %d\n", runningPCB->readyTime, runningPCB->pid);

    pushPCB(runningPCB);

    if (readyPCB == NULL) {
        TracePrintf(LOG, "Switch to idle\n");
        ContextSwitch(MySwitchFunc, &runningPCB->ctx, (void *)runningPCB, (void *)idlePCB);
    } else {
        ContextSwitch(MySwitchFunc, &runningPCB->ctx, (void *)runningPCB, (void *)popPCB(READY));
    }

    return 0;
}

int TtyRead(int tty_id, void *buf, int len) {
    return 0;
}

int TtyWrite(int tty_id, void *buf, int len) {
    return 0;
}