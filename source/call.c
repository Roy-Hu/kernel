#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <comp421/yalnix.h>


#include "pcb.h"
#include "pagetable.h"
#include "global.h"

int Fork(void) {
    // not enough physical pages for child process
    if (!check_enough_pages_fork()) return ERROR;

    PCB* child = (PCB*)malloc(sizeof(PCB));
    child->ctx = (SavedContext*)malloc(sizeof(SavedContext));

    child->parent = runningPCB;
    child->state = RUNNING;
    child->pid = processId++;
    child->ptr0 = allocateNewPage();

    TracePrintf(LOG, "allocated new address for child in ptr0: %p!\n", child->ptr0);
    TracePrintf(LOG, "RunningPCB ptr0: %x!\n", runningPCB->ptr0);

    child->readyTime = clocktime;
    child->next = NULL;
    child->brk = runningPCB->brk;

    ContextSwitch(switch_fork,runningPCB->ctx, (void*)runningPCB, (void*) child);

    if (runningPCB == child) return 0;
    
    else return child->pid;
}

int Exec(char *filename, char **argvec) {
    return 0;
}

void Exit(int status) {
    return;
}

int Wait(int *status_ptr) {
    return 0;
}

int GetPid(void) {
    return runningPCB->pid;
}

int Brk(void *addr) {
    unsigned long break_vpn = UP_TO_PAGE((unsigned long)runningPCB->brk) >> PAGESHIFT;
    unsigned long addr_vpn = UP_TO_PAGE((unsigned long)addr) >> PAGESHIFT;
    
    /*Deallocating ptes*/
    if (break_vpn > addr_vpn) {
        TracePrintf(LOG, "Deallocating %d to %d\n", addr_vpn, break_vpn);
        unsigned long i = break_vpn;
        for (; i > addr_vpn; --i) {
            if (runningPCB->ptr0[i].valid) {
                runningPCB->ptr0[i].valid = 0;
                freePhysicalFrame(runningPCB->ptr0[i].pfn);
            }
        }
    } else {
        // TracePrintf(LOG, "Allocating %d to %d\n", break_vpn, addr_vpn);
        TracePrintf(TRC, "freePFN %d\n",physicalFrames.freePFN);
        
        if (addr_vpn - break_vpn > physicalFrames.freePFN) return ERROR;
        // TracePrintf(LOG, "test\n");
        // why MEM_INVALID_PAGES to addr_vpn instead of break_vpn to addr_vpn?
        unsigned long i = MEM_INVALID_PAGES;
        for (; i < addr_vpn; ++i)
           
            if (!runningPCB->ptr0[i].valid) {
                // TracePrintf(LOG, "test%d\n", i);
                int pfn = getFreePhysicalFrame();
                if (pfn == -1) {
                    return ERROR;
                }
                
                setPTE(&runningPCB->ptr0[i], pfn, 1, (PROT_READ | PROT_WRITE), (PROT_READ | PROT_WRITE));
            }
    }
    // TracePrintf(LOG, "Exit");
    runningPCB->brk = (unsigned long) addr;

    return 0;
}

int Delay (int clock_ticks) {
    if (clock_ticks < 0) return ERROR;
    runningPCB->readyTime = clocktime + clock_ticks;
    runningPCB->state = WAITING;

    TracePrintf(LOG, "Delay %d for pid %d\n", runningPCB->readyTime, runningPCB->pid);

    pushPCB(runningPCB);

    if (readyPCB == NULL) {
        /*No current readyPCB, switch to idle*/
        TracePrintf(LOG, "Switch to idle\n");
        ContextSwitch(switch_func, runningPCB->ctx, (void *)runningPCB, (void *)idlePCB);
    } else {
        TracePrintf(LOG, "Switch to Process pid: %d\n", readyPCB->pid);
        ContextSwitch(switch_func, runningPCB->ctx, (void *)runningPCB, (void *)popPCB(READY));
    }

    return 0;
}

int TtyRead(int tty_id, void *buf, int len) {
    return 0;
}

int TtyWrite(int tty_id, void *buf, int len) {
    return 0;
}