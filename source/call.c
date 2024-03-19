#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <comp421/yalnix.h>


#include "pcb.h"
#include "pagetable.h"
#include "global.h"
#include "call.h"

int MyFork(void) {
    // not enough physical pages for child process
    if (!check_enough_pages_fork()) return ERROR;

    PCB* child = (PCB*)malloc(sizeof(PCB));
    child->ctx = (SavedContext*)malloc(sizeof(SavedContext));

    child->parent = runningPCB;

    if (runningPCB->child == NULL) runningPCB->child = child;
    else addSibling(runningPCB->child, child);

    runningPCB->childNum++;
    TracePrintf(LOG, "PID %d call Fork, now it have %d child\n", runningPCB->pid, runningPCB->childNum);

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

int MyExec(char *filename, char **argvec, ExceptionInfo *info) {
    TracePrintf(LOG, "PID %d call Exec\n", runningPCB->pid);
    
    unsigned long vpn = (unsigned long)filename >> PAGESHIFT;
    int eof = 0;
    int i;

    // check if filename is valid in pt
    while (eof == 0) {
        if (runningPCB->ptr0[vpn].valid == 0 || !(runningPCB->ptr0[vpn].kprot & PROT_READ)) {
            TracePrintf(ERR, "Invalid filename\n");
            return ERROR;
        }

        // check if filename is null-terminated
        for (i = 0; i < ((vpn + 1) << PAGESHIFT) - (unsigned long)filename; i++) {
            if (filename[i] == '\0') {
                eof = 1;
                break;
            }
        }

        // filename continues to next page
        vpn++;
    }

    // check if argvec is valid in pt
    vpn = (unsigned long)argvec >> PAGESHIFT;
    eof = 0;

    while (eof == 0) {
        if (runningPCB->ptr0[vpn].valid == 0 || !(runningPCB->ptr0[vpn].kprot & PROT_READ)) {
            TracePrintf(ERR, "Invalid argvec\n");
            return ERROR;
        }

        // check if argvec[i] is NULL
        for (i = 0; i * sizeof(char*) < ((vpn + 1) << PAGESHIFT) - (unsigned long)argvec; i++) {
            if (argvec[i] == NULL) {
                eof = 1;
                break;
            }
        }
        // argvec continues to next page
        vpn++;
    }

    TracePrintf(LOG, "Exec filename: %s\n", filename);
    int status = LoadProgram(filename, argvec, info);

    if (status != 0) {
        TracePrintf(ERR, "LoadProgram failed\n");
        return ERROR;
    }
    
    return 0;
}

void MyExit(int status) {
    TracePrintf(LOG, "PID %d call Exit\n", runningPCB->pid);

    terminateProcess(runningPCB, status);

    ContextSwitch(switch_func, runningPCB->ctx, (void *)runningPCB, (void *)popPCB(READY));

    while(1){}
}

int MyWait(int *status_ptr) {
    TracePrintf(LOG, "PID %d call Wait\n", runningPCB->pid);

    unsigned long vpn = (unsigned long)((void *)status_ptr + sizeof(int)) >> PAGESHIFT;
    
    int i;

    // Check if status_ptr is valid in pt
    for (i = vpn; i < (UP_TO_PAGE((unsigned long)status_ptr) >> PAGESHIFT); i++) {
        if (runningPCB->ptr0[i].valid == 0 || !(runningPCB->ptr0[i].kprot & PROT_WRITE)) {
            TracePrintf(ERR, "Invalid status_ptr\n");
            return ERROR;
        }
    }
    
    if (runningPCB->exitChild == NULL) {
        if (runningPCB->childNum == 0) {
            TracePrintf(ERR, "pid %d No children to wait for\n", runningPCB->pid);
            return ERROR;
        }

        runningPCB->state = WAITCHILD;
        ContextSwitch(switch_func, runningPCB->ctx, (void *)runningPCB, (void *)popPCB(READY));
    }

    exitChildStatus *exitChild = popExitStatus(runningPCB);

    *status_ptr = exitChild->status;
    int pid = exitChild->pid;

    free(exitChild);

    TracePrintf(LOG, "pid %d wait for pid %d, status %d\n", runningPCB->pid, pid, *status_ptr);

    return pid;
}

int MyGetPid(void) {
    return runningPCB->pid;
}

int MyBrk(void *addr) {
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

int MyDelay (int clock_ticks) {
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

int MyTtyRead(int tty_id, void *buf, int len) {
    return 0;
}

int MyTtyWrite(int tty_id, void *buf, int len) {
    return 0;
}