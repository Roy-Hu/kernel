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
        info->regs[0] = Fork();
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
        TracePrintf(LOG, "Trap\n");
        info->regs[0] = Brk((void*)(info->regs[1]));
        break;
    case YALNIX_DELAY:  
        info->regs[0] = Delay(info->regs[1]);
        break;
    case YALNIX_TTY_READ:
        /*code*/
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
    TracePrintf(LOG, "previous \n");

    /*There are waiting processes, push the waiting process to ready if reached
      time */
    if (waitingPCB != NULL) {
        TracePrintf(LOG, "Enter first if\n");
        while (waitingPCB != NULL && waitingPCB->readyTime <= clocktime) {
            flag = true;
            PCB *pcb = popPCB(WAITING);
            pcb->state = READY;
            pushPCB(pcb);
        }

    }
    TracePrintf(LOG, "Exit first if\n");
    // if (runningPCB == idlePCB && flag) {
    //     TracePrintf(LOG, "Enter second if\n");
    PCB *p = popPCB(READY);
    // No ready PCB as for now
    if (p != NULL)
        TracePrintf(LOG, "Popped PCB pid: %d\n", p->pid);
    ContextSwitch(switch_clock_trap, runningPCB->ctx, (void *)runningPCB, p);
}

void TrapIllegalHandler(ExceptionInfo *info) {
    TracePrintf(LOG, "TrapIllegalHandler\n");
    Halt();
}

void TrapMemoryHandler(ExceptionInfo *info) {
    TracePrintf(LOG, "TrapMemoryHandler\n");
    void* addr = info->addr;
    unsigned long brk_vpn = UP_TO_PAGE(runningPCB->brk) >> PAGESHIFT;
    unsigned long tar_vpn = DOWN_TO_PAGE(addr) >> PAGESHIFT;
    int stk_bound_vpn = user_stack_vpn();
    /*address to acquire exceed user stack*/
    //if (tar_vpn >= stk_bound_vpn) return ERROR;
    if (tar_vpn > brk_vpn && (stk_bound_vpn - brk_vpn) < physicalFrames.freePFN && tar_vpn <= stk_bound_vpn) {
        int i = tar_vpn;
        TracePrintf(LOG, "Set user_stack vpn from: %d to %d\n",stk_bound_vpn, tar_vpn);
        for (;i <= stk_bound_vpn; ++i) {
            int pfn = getFreePhysicalFrame();
            setPTE(&runningPCB->ptr0[i], pfn, 1, (PROT_READ|PROT_WRITE), (PROT_READ|PROT_WRITE));
        }
    }
    else {
        TracePrintf(LOG, "ptr_0 is: %p\n", runningPCB->ptr0);
        TracePrintf(LOG, "unassigned memory on addr vpn: %d\t stack pointer addr: %d\n", tar_vpn,stk_bound_vpn);
        return Halt();
    }
    // Halt();
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