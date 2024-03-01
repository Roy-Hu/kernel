#include "trap.h"
#include "global.h"

#include <comp421/yalnix.h>

void TrapKernelHandler(ExceptionInfo *info) {
    TracePrintf(LOG, "TrapKernelHandler\n");

    switch (info->code)
    {
    case YALNIX_FORK:
        /* code */
        break;
    case YALNIX_EXEC:
        /* code */
        break;
    case YALNIX_EXIT:
        /* code */
        break;
    case YALNIX_WAIT:
        /* code */
        break;
    case YALNIX_GETPID: 
        info->regs[0] = GetPid();
        TracePrintf(LOG, "GetPid %d\n", currentPCB->pid);

        break;
    case YALNIX_BRK:
        /* code */
        break;
    case YALNIX_DELAY:  
        /* code */
        break;
    case YALNIX_TTY_READ:
        /* code */
        break;
    case YALNIX_TTY_WRITE:  
        /* code */
        break;

    
    default:
        break;
    }
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