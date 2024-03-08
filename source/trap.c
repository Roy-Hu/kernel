#include <stddef.h>
#include <stdbool.h>

#include <comp421/yalnix.h>

#include "trap.h"
#include "pcb.h"
#include "global.h"

TrapHandlerPtr interruptVectorTable[TRAP_VECTOR_SIZE];

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
        TracePrintf(LOG, "GetPid %d\n", runningPCB->pid);

        break;
    case YALNIX_BRK:
        /* code */
        break;
    case YALNIX_DELAY:  
        Delay(info->regs[1]);
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
    TracePrintf(LOG, "TrapClockHandler\n");
    clocktime++;
    bool flag = false;

    if (waitingPCB != NULL) {

        while (waitingPCB != NULL && waitingPCB->readyTime <= clocktime) {
            flag = true;
            PCB *pcb = popPCB(WAITING);
            pcb->state = READY;
            pushPCB(pcb);
        }

    }

    if (runningPCB == idlePCB && flag) {
        ContextSwitch(MySwitchFunc, &runningPCB->ctx, (void *)runningPCB, (void *)popPCB(READY));
    }
}

void TrapIllegalHandler(ExceptionInfo *info) {
    TracePrintf(LOG, "TrapIllegalHandler\n");
    Halt();
}

void TrapMemoryHandler(ExceptionInfo *info) {
    TracePrintf(LOG, "TrapMemoryHandler\n");
    Halt();
}

void TrapMathHandler(ExceptionInfo *info) {
    TracePrintf(LOG, "TrapMathHandler\n");
    Halt();
}

void TrapTtyReceiveHandler(ExceptionInfo *info) {
    TracePrintf(LOG, "TrapTtyReceiveHandler\n");
    Halt();
}   

void TrapTtyTransmitHandler(ExceptionInfo *info) {
    TracePrintf(LOG, "TrapTtyTransmitHandler\n");
    Halt();
}