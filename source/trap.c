#include <stddef.h>
#include <stdbool.h>
#include <comp421/yalnix.h>

#include "call.h"
#include "trap.h"
#include "pcb.h"
#include "global.h"

TrapHandlerPtr interruptVectorTable[TRAP_VECTOR_SIZE];

void TrapKernelHandler(ExceptionInfo *info) {
    TracePrintf(LOG, "TrapKernelHandler\n");

    switch (info->code)
    {
    case YALNIX_FORK:
        info->regs[0] = MyFork();
        break;
    case YALNIX_EXEC:
        MyExec((char *)info->regs[1], (char **)info->regs[2], info);
        break;
    case YALNIX_EXIT:
        TracePrintf(LOG, "Exit %d\n", runningPCB->pid);
        MyExit((int)info->regs[1]);
        break;
    case YALNIX_WAIT:
        TracePrintf(LOG, "Wait %d\n", runningPCB->pid);
        info->regs[0] = MyWait((int *)info->regs[1]);
        break;
    case YALNIX_GETPID: 
        info->regs[0] = MyGetPid();
        TracePrintf(LOG, "GetPid %d\n", runningPCB->pid);
        break;
    case YALNIX_BRK:
        TracePrintf(LOG, "Brk\n");
        info->regs[0] = MyBrk((void*)(info->regs[1]));
        break;
    case YALNIX_DELAY:  
        TracePrintf(LOG, "Delay\n");
        info->regs[0] = MyDelay(info->regs[1]);
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
    // bool flag = false;
    TracePrintf(TRC, "previous \n");

    /*There are waiting processes, push the waiting process to ready if reached
      time */
    if (waitingPCB != NULL) {
        TracePrintf(TRC, "Enter first if\n");
        while (waitingPCB != NULL && waitingPCB->readyTime <= clocktime) {
            // flag = true;
            PCB *pcb = popPCB(WAITING);
            pcb->state = READY;
            pushPCB(pcb);
        }
    }

    // TracePrintf(LOG, "Exit first if\n");
    // if (runningPCB == idlePCB && flag) {
    //     TracePrintf(LOG, "Enter second if\n");
    PCB *ready = popPCB(READY);

    if (ready != NULL) TracePrintf(LOG, "Wake PCB pid: %d\n", ready->pid);
        
    ContextSwitch(switch_clock_trap, runningPCB->ctx, (void *)runningPCB, ready);
}

void TrapIllegalHandler(ExceptionInfo *info) {
    TracePrintf(LOG, "TrapIllegalHandler\n");
    Halt();
}

void TrapMemoryHandler(ExceptionInfo *info) {
    TracePrintf(LOG, "TrapMemoryHandler\n");
    void *addr = info->addr;

    unsigned long brk_vpn = UP_TO_PAGE(runningPCB->brk) >> PAGESHIFT;
    unsigned long trap_vpn = DOWN_TO_PAGE(addr) >> PAGESHIFT;
    int stk_bound_vpn = user_stack_vpn();

    /*address to acquire exceed user stack*/
    //if (trap_vpn >= stk_bound_vpn) return ERROR;
    // why stk_bound_vpn - brk_vpn? isn't it stk_bound_vpn - trap_vpn?
    if (trap_vpn > brk_vpn && (stk_bound_vpn - brk_vpn) < physicalFrames.freePFN && trap_vpn <= stk_bound_vpn) {
        int i = trap_vpn;
        TracePrintf(LOG, "Set user_stack vpn from: %d to %d\n",stk_bound_vpn, trap_vpn);
        for (;i <= stk_bound_vpn; ++i) {
            int pfn = getFreePhysicalFrame();
            if (pfn == -1) {
                return Halt();
            }

            setPTE(&runningPCB->ptr0[i], pfn, 1, (PROT_READ | PROT_WRITE), (PROT_READ | PROT_WRITE));
        }
    } else {
        switch (info->code)
        {
        case TRAP_MEMORY_MAPERR:
            TracePrintf(LOG, "TRAP_MEMORY_MAPERR: No mapping at addr %p\n", info->addr);
            break;
        case TRAP_MEMORY_ACCERR:
            TracePrintf(LOG, "TRAP_MEMORY_ACCERR: Protection violation at addr %p\n", info->addr);
            break;
        case TRAP_MEMORY_KERNEL:
            TracePrintf(LOG, "TRAP_MEMORY_KERNEL: Linux kernel sent SIGSEGV at addr %p\n", info->addr);
            break;
        case TRAP_MEMORY_USER:
            TracePrintf(LOG, "TRAP_MEMORY_USER: Received SIGSEGV from user %p\n", info->addr);
            break;
        default:
            break;
        }

        TracePrintf(LOG, "ptr_0 is: %p\n", runningPCB->ptr0);
        TracePrintf(LOG, "unassigned memory on addr vpn: %d\n", trap_vpn);
        TracePrintf(LOG, "stack pointer addr: %d\n",stk_bound_vpn);
        TracePrintf(LOG, "brk addr %p, brk vpn %d\n", runningPCB->brk, brk_vpn);

        MyExit(ERROR);

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