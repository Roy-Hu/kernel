#include <stddef.h>
#include <stdlib.h>
#include "pagetable.h"
#include "pcb.h"
#include "global.h"

PCB *runningPCB, *waitingPCB, *readyPCB, *idlePCB, *initPCB;

PCB *readyPCBTail, *waitingPCBTail;

char swapBuff[PAGESIZE * KERNEL_STACK_PAGES];

void terminateProcess(PCB *pcb, int status) {
    TracePrintf(LOG, "Terminate Process %d\n", pcb->pid);
    pcb->state = TERMINATED;

    if (pcb->parent != NULL) {
        pushExitStatus(pcb->parent, pcb->pid, status);
        pcb->parent->childNum--;

        removeSibling(pcb->parent, pcb);

        if (pcb->parent->state == WAITCHILD) {
            pcb->parent->state = READY;
            pushPCB(pcb->parent);
        }

        pcb->parent = NULL;
    }

    // remove parent from all child and sibling
    if (pcb->child != NULL) {
        PCB *cur = pcb->child;
        while (cur != NULL) {
            cur->parent = NULL;
            cur = cur->sibling;
        }
    }
}

SavedContext *test_init(SavedContext *ctxp, void *p1, void *p2) {
    PCB *pcb1 = (PCB *)p1;
    PCB *pcb2 = (PCB *)p2;
    /*copy kernel stack to init*/
    memcpy(swapBuff, (void*)KERNEL_STACK_BASE, PAGESIZE * KERNEL_STACK_PAGES);
    //WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

    int vpn = ((unsigned long)pcb2->ptr0 - VMEM_1_BASE) >> PAGESHIFT;
    unsigned long paddr = (RCS421RegVal)(ptr1[vpn].pfn << PAGESHIFT) | (((unsigned long)pcb2->ptr0) & PAGEOFFSET);
    pcb2->pt0addr = paddr;

    WriteRegister(REG_PTR0,(RCS421RegVal) paddr);

    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

    TracePrintf(INF, "New ptr0 %p\n", (void*)pcb2->ptr0);

    // copy pcb1's kernel stack to pcb2's kernel stack?
    memcpy((void*)KERNEL_STACK_BASE, swapBuff, PAGESIZE * KERNEL_STACK_PAGES);
    memcpy(pcb2->ctx, pcb1->ctx, sizeof(SavedContext));

    runningPCB = pcb2;
    runningPCB->state = RUNNING;

    TracePrintf(TRC, "Arrived Here\n");

    return pcb2->ctx;
}

SavedContext *switch_func(SavedContext *ctxp, void *p1, void *p2) {
    TracePrintf(LOG, "Enter context switch!\n");
    PCB *pcb1 = (PCB *)p1;
    PCB *pcb2 = (PCB *)p2;

    if (pcb1->state == TERMINATED) {
        TracePrintf(LOG, "Process %d is terminated\n", pcb1->pid);
        int i;
        for (i = MEM_INVALID_PAGES; i < PAGE_TABLE_LEN; i++) {
            if (pcb1->ptr0[i].valid) {
                freePhysicalFrame(pcb1->ptr0[i].pfn);
            }
        }

        freeExitStatus(pcb1);

        // TODO: free pcb1->ptr0
        // free(pcb1->ptr0);
        free(pcb1->ctx);
        free(pcb1);
    }

    if (pcb2 == NULL) {
        /*Switch to idle*/
        unsigned long vpn = DOWN_TO_PAGE((void *)idlePCB->ptr0 - VMEM_1_BASE) >> PAGESHIFT;
        unsigned long paddr = (ptr1[vpn].pfn << PAGESHIFT) | (((unsigned long)idlePCB->ptr0) & PAGEOFFSET);

        WriteRegister(REG_PTR0,(RCS421RegVal)paddr);
        WriteRegister(REG_TLB_FLUSH,TLB_FLUSH_0);

        runningPCB = idlePCB;
        runningPCB->state = RUNNING;

        TracePrintf(LOG, "Currently no readyPCB, Switch to Idle\n");

        return idlePCB->ctx;
    } else {
        TracePrintf(LOG, "Context Switch to %d Process!\n", pcb2->pid);

        unsigned long vpn = DOWN_TO_PAGE((void *)pcb2->ptr0 - VMEM_1_BASE) >> PAGESHIFT;
        unsigned long paddr = (ptr1[vpn].pfn << PAGESHIFT) | (((unsigned long)pcb2->ptr0) & PAGEOFFSET);

        WriteRegister(REG_PTR0,(RCS421RegVal)paddr);
        WriteRegister(REG_TLB_FLUSH,TLB_FLUSH_0);

        runningPCB = pcb2;
        runningPCB->state = RUNNING;

        TracePrintf(TRC, "Return from switch_func\n");

        return pcb2->ctx;
    }
}

/*
 *if no p2, return p1.
 *if there is p2, push p1 to ready queue
 *if p1 is not idle.
 *if it is idle, just continuing executing p1
 */
SavedContext *switch_clock_trap(SavedContext *ctxp, void *p1, void *p2) {
    TracePrintf(LOG, "Switch Clock Trap\n");

    PCB *pcb1 = (PCB *)p1;
    PCB *pcb2 = (PCB *)p2;

    if (pcb2 == NULL) return pcb1->ctx;
    
    TracePrintf(LOG, "Switch to process: %d\n", pcb2->pid);
    
    unsigned long vpn = DOWN_TO_PAGE((void *)pcb2->ptr0 - VMEM_1_BASE) >> PAGESHIFT;
    unsigned long paddr = (ptr1[vpn].pfn << PAGESHIFT) | (((int)pcb2->ptr0) & PAGEOFFSET);
    
    WriteRegister(REG_PTR0,(RCS421RegVal)paddr);
    WriteRegister(REG_TLB_FLUSH,TLB_FLUSH_0);

    pcb1->state = READY;

    if (pcb1 != idlePCB) pushPCB(pcb1);

    runningPCB=pcb2;
    
    return pcb2->ctx;
}

/**
 * switch function for fork call
 * p1: running process
 * p2: child process
 * 
 * **/
SavedContext *switch_fork(SavedContext *ctxp, void *p1, void *p2) {
    PCB *pcb1 = (PCB *)p1;
    PCB *pcb2 = (PCB *)p2;
    int i;

    TracePrintf(LOG, "Enter Fork Context Switch Call!\n");
    /*need to check for enough memory first*/
    /*copy PTR from parent to child*/
    for (i = 0; i < PAGE_TABLE_LEN; ++i) {
        pcb2->ptr0[i].valid = 0;
        if (pcb1->ptr0[i].valid == 1) {
            int pfn = getFreePhysicalFrame();
            if (pfn == -1) {
                return NULL;
            }

            setPTE(&pcb2->ptr0[i], pfn, 1, pcb1->ptr0[i].uprot, pcb1->ptr0[i].kprot);
            TracePrintf(TRC, "set new pte for pcb2: %d\n", i);
        }
    }

    TracePrintf(LOG, "finished copying ptr0 of pcb1 to child process!\n");

    /*find an unsed ptr0 from parent*/
    int brk_vpn = UP_TO_PAGE(USER_STACK_LIMIT) >> PAGESHIFT;

    TracePrintf(TRC, "new brk_vpn for swap: %d! And its valid bit is: %d\n",brk_vpn, pcb1->ptr0[brk_vpn].valid);
    // Copy page table entries's content from parent to child
    for (i = 0; i < PAGE_TABLE_LEN; ++i) {
        if (pcb2->ptr0[i].valid == 1) {
            // pcb1->ptr0[brk_vpn] is a free PTE, use it to point to pcb2->ptr0[i].pfn for copying pcb1->ptr0[i]'s
            // content to pcb2->ptr0[i]
            setPTE(&pcb1->ptr0[brk_vpn], pcb2->ptr0[i].pfn, 1, PROT_NONE, (PROT_READ | PROT_WRITE));
            WriteRegister(REG_TLB_FLUSH, (RCS421RegVal)(brk_vpn << PAGESHIFT));
            memcpy((void *)(brk_vpn << PAGESHIFT), (void *)(i << PAGESHIFT), PAGESIZE);
            //TracePrintf(LOG, "Mem copy setup successfully!\n");
        }

        pcb1->ptr0[brk_vpn].valid = 0;
        WriteRegister(REG_TLB_FLUSH, (RCS421RegVal)(brk_vpn<<PAGESHIFT));
    }
    TracePrintf(LOG, "finished copying content of the memory!\n");

    unsigned long vpn = DOWN_TO_PAGE((void *)pcb2->ptr0 - VMEM_1_BASE) >> PAGESHIFT;
    unsigned long paddr = (ptr1[vpn].pfn << PAGESHIFT) | (((int)pcb2->ptr0) & PAGEOFFSET);

    WriteRegister(REG_PTR0, (RCS421RegVal)paddr);
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

    memcpy(pcb2->ctx, pcb1->ctx, sizeof(SavedContext));
    pcb1->state=READY;

    pushPCB(pcb1);
    runningPCB = pcb2;
    
    return pcb2->ctx;
}

PCB *createPCB(int pid) {
    TracePrintf(LOG, "Create New PCB %d\n", pid);

    PCB *pcb = (PCB *)malloc(sizeof(PCB));
    pcb->readyTime = clocktime;
    pcb->pid = pid;
    pcb->parent = NULL;
    pcb->next = NULL;
    pcb->ctx = (SavedContext*)malloc(sizeof(SavedContext));

    if (pid == IDLE_PID) {
        TracePrintf(LOG, "Create Idle PCB\n");
        // Why the first process is idle instead of init?
        pcb->pt0addr = INIT_PT0_PFN << PAGESHIFT;
        /* address of the idle process*/
        pcb->ptr0 = ptr0;
        pcb->brk = MEM_INVALID_PAGES;
    }
    else if (pid == INIT_PID) {
        TracePrintf(LOG, "Create Init PCB\n");
        pcb->brk = MEM_INVALID_PAGES;
        pcb->ptr0 = allocateNewPage();
        // pcb->pt0addr = pcb->ptr0 << PAGESHIFT;
        /*mark the page table  entry as valid (int use) for kernel stack*/
        int addr;
        for(addr = KERNEL_STACK_BASE; addr < KERNEL_STACK_LIMIT; addr += PAGESIZE) {
            int pfn = getFreePhysicalFrame();
            if (pfn == -1) {
                return NULL;
            }

            setPTE(&pcb->ptr0[addr >> PAGESHIFT], pfn, 1, PROT_NONE, (PROT_READ | PROT_WRITE));
        }

        TracePrintf(TRC, "Exit else if INIT Process\n");
    }
   
    return pcb;
}

void addSibling(PCB *parent, PCB *sibling) {
    PCB *cur = parent->child;
    /* find the last sibling */
    while (cur->sibling != NULL) {
        cur = cur->sibling;
    }
    cur->sibling = sibling;
}

void removeSibling(PCB *parent, PCB *child) {
    /* remove a sibling */
    PCB *cur = parent->child;
    if (cur == child) {
        parent->child = cur->sibling;
        return;
    }
    /* find the child to be deleted */
    while (cur->sibling != child) {
        cur = cur->sibling;
    }

    cur->sibling = child->sibling;
}

void pushExitStatus(PCB *pcb, int pid, int status) {
    exitChildStatus *newStatus = (exitChildStatus *)malloc(sizeof(exitChildStatus));
    newStatus->pid = pid;
    newStatus->status = status;
    newStatus->next = NULL;

    if (pcb->exitChild == NULL) {
        pcb->exitChild = newStatus;
    } else {
        exitChildStatus *cur = pcb->exitChild;
        while (cur->next != NULL) {
            cur = cur->next;
        }
        cur->next = newStatus;
    }
}

exitChildStatus *popExitStatus(PCB *pcb) {
    if (pcb->exitChild == NULL) {
        return NULL;
    } else {
        // remember to free the memory
        exitChildStatus *cur = pcb->exitChild;
        pcb->exitChild = cur->next;
        return cur;
    }
}

void freeExitStatus(PCB *pcb) {
    exitChildStatus *cur = pcb->exitChild;
    while (cur != NULL) {
        exitChildStatus *next = cur->next;
        free(cur);
        cur = next;
    }
}

void pushPCB(PCB *pcb) {
    switch (pcb->state)
    {
    case WAITING:
        TracePrintf(INF, "Push PCB %d to WAITING\n", pcb->pid);

        PCB *cur = waitingPCB;
        while (cur != NULL) {
            if (cur->readyTime > pcb->readyTime) {
                if (cur == waitingPCB) {
                    pcb->next = waitingPCB;
                    waitingPCB = pcb;
                } else {
                    pcb->next = cur->next;
                    cur->next = pcb;
                }

                return;
            } else if (cur->next == NULL) {
                cur->next = pcb;
                waitingPCBTail = pcb;
                return;
            } else {
                cur = cur->next;
            }
        }
        /*no waiting pcb*/
        waitingPCB =pcb;
        waitingPCBTail = pcb;
        waitingPCB->next = NULL;

        break;
    case READY:
        TracePrintf(INF, "Push PCB %d to READY\n", pcb->pid);

        if (readyPCB == NULL) {
            readyPCB = pcb;
            readyPCBTail = pcb;
        } else {
            readyPCBTail->next = pcb;
            readyPCBTail = pcb;
        }

        break;
    default:
        TracePrintf(ERR, "Invalid state\n");
        break;
    }
}

PCB *popPCB(STATE state) {
    PCB *rtn;
    switch (state)
    {
    case WAITING:
        rtn = waitingPCB;
        rtn->next == NULL;

        if (waitingPCB == waitingPCBTail) {
            waitingPCB = NULL;
            waitingPCBTail = NULL;
        } else waitingPCB = waitingPCB->next;
        
        TracePrintf(INF, "Pop PCB %d from WAITING\n", rtn->pid);

        return rtn;
    case READY:
        /*No Ready PCB*/
        if (readyPCB == NULL) return NULL;
        /*Has Ready PCB*/
        rtn = readyPCB;
        rtn->next == NULL;

        if (readyPCB == readyPCBTail) {
            readyPCB = NULL;
            readyPCBTail = NULL;
        } else readyPCB = readyPCB->next;
        
        TracePrintf(INF, "Pop PCB %d from READY\n", rtn->pid);

        return rtn;
    default:
        break;
    }
}
