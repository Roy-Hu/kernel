#include "trap.h"
#include "global.h"

void TrapKernelHandler(ExceptionInfo *info) {
    TracePrintf(LOG, "TrapKernelHandler\n");

    Halt();
}

void TrapClockHandler(ExceptionInfo *info) {
    TracePrintf(1, "TrapClockHandler\n");
    Halt();
}

void TrapIllegalHandler(ExceptionInfo *info) {
    TracePrintf(1, "TrapIllegalHandler\n");
    Halt();
}

void TrapMemoryHandler(ExceptionInfo *info) {
    TracePrintf(1, "TrapMemoryHandler\n");
    Halt();
}

void TrapMathHandler(ExceptionInfo *info) {
    TracePrintf(1, "TrapMathHandler\n");
    Halt();
}

void TrapTtyReceiveHandler(ExceptionInfo *info) {
    TracePrintf(1, "TrapTtyReceiveHandler\n");
    Halt();
}   

void TrapTtyTransmitHandler(ExceptionInfo *info) {
    TracePrintf(1, "TrapTtyTransmitHandler\n");
    Halt();
}