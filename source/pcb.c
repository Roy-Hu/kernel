#include <stddef.h>
#include <stdlib.h>
#include "pagetable.h"
#include "pcb.h"
#include "global.h"

PCB *runningPCB, *waitingPCB, *readyPCB, *idlePCB, *initPCB;

PCB *readyPCBTail, *waitingPCBTail;

char swapBuff[PAGESIZE * KERNEL_STACK_PAGES];


SavedContext *test_init(SavedContext *ctxp, void *p1, void *p2) {
    PCB *pcb1 = (PCB *)p1;
    PCB *pcb2 = (PCB *)p2;
    /*copy kernel stack to init*/
    memcpy(swapBuff,(void*)KERNEL_STACK_BASE,PAGESIZE*KERNEL_STACK_PAGES);
    //WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
    int vpn = ((unsigned long)pcb2->ptr0 - VMEM_1_BASE) >> PAGESHIFT;
    unsigned long paddr = (RCS421RegVal)(ptr1[vpn].pfn << PAGESHIFT) | (((unsigned long)pcb2->ptr0)&PAGEOFFSET);
    pcb2->pt0addr = paddr;

    WriteRegister(REG_PTR0,(RCS421RegVal) paddr);

    TracePrintf(LOG, "New ptr0 %p\n", (void*)pcb2->ptr0);

    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
    TracePrintf(LOG, "Arrived Here\n");
    memcpy((void*)KERNEL_STACK_BASE,swapBuff,PAGESIZE*KERNEL_STACK_PAGES);
    memcpy(pcb2->ctx, pcb1->ctx, sizeof(SavedContext));
    runningPCB=pcb2;
    runningPCB->state = RUNNING;
    TracePrintf(LOG, "Arrived Here\n");
    return pcb2->ctx;

    
}

SavedContext *switch_func(SavedContext *ctxp, void *p1, void *p2) {
    TracePrintf(LOG, "Enter Delay context switch!\n");
    PCB *pcb1 = (PCB *)p1;
    PCB *pcb2 = (PCB *)p2;
    
    if (pcb2 == NULL) {
        /*Switch to idle*/
        unsigned long vpn = DOWN_TO_PAGE((void *)idlePCB->ptr0 - VMEM_1_BASE) >> PAGESHIFT;
        unsigned long paddr = (ptr1[vpn].pfn << PAGESHIFT) | (((int)idlePCB->ptr0)&PAGEOFFSET);
        WriteRegister(REG_PTR0,(RCS421RegVal)paddr);
        WriteRegister(REG_TLB_FLUSH,TLB_FLUSH_0);
        runningPCB = idlePCB;
        runningPCB->state = RUNNING;
        TracePrintf(LOG, "Currently no readyPCB, Switch to Idle\n");
        return idlePCB->ctx;
    }
    else {
        TracePrintf(LOG, "Context Switch to %d Process!\n", pcb2->pid);
        unsigned long vpn = DOWN_TO_PAGE((void *)pcb2->ptr0 - VMEM_1_BASE) >> PAGESHIFT;
        unsigned long paddr = (ptr1[vpn].pfn << PAGESHIFT) | (((int)pcb2->ptr0)&PAGEOFFSET);
        WriteRegister(REG_PTR0,(RCS421RegVal)paddr);
        WriteRegister(REG_TLB_FLUSH,TLB_FLUSH_0);
        runningPCB=pcb2;
        runningPCB->state = RUNNING;
        TracePrintf(LOG, "Delay Here\n");
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
    PCB *pcb1 = (PCB *)p1;
    PCB *pcb2 = (PCB *)p2;
    if (pcb2 == NULL) return pcb1->ctx;
    else TracePrintf(LOG, "Switch to process: %d!\n", pcb2->pid);
    
    unsigned long vpn = DOWN_TO_PAGE((void *)pcb2->ptr0 - VMEM_1_BASE) >> PAGESHIFT;
    unsigned long paddr = (ptr1[vpn].pfn << PAGESHIFT) | (((int)pcb2->ptr0)&PAGEOFFSET);
    
    WriteRegister(REG_PTR0,(RCS421RegVal)paddr);
    WriteRegister(REG_TLB_FLUSH,TLB_FLUSH_0);
    pcb1->state = READY;
    if(pcb1 != idlePCB) pushPCB(pcb1);
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
    int i = 0;
    TracePrintf(LOG, "Enter Fork Call!\n");
    /*copy pet from parent to child*/
    for (;i < PAGE_TABLE_LEN; ++i) {
        if (pcb1->ptr0[i].valid == 1) {
            int pfn = getFreePhysicalFrame();
            setPTE(&pcb2->ptr0[i], pfn, pcb1->ptr0[i].valid, pcb1->ptr0[i].uprot, pcb1->ptr0[i].kprot);
            TracePrintf(LOG, "set new pte for pcb2: %d\n", i);
        }
    }
     TracePrintf(LOG, "finished copying ptr0 of pcb1 to child process!\n");
    /*find an unsed ptr0 from parent*/
    int brk_vpn = UP_TO_PAGE(USER_STACK_LIMIT) >> PAGESHIFT;
    TracePrintf(LOG, "new brk_vpn for swap: %d! And its valid bit is: %d\n",brk_vpn, pcb1->ptr0[brk_vpn].valid);
    for (i = 0; i < PAGE_TABLE_LEN; ++i) {
        if (pcb2->ptr0[i].valid == 1) {
            setPTE(&pcb1->ptr0[brk_vpn], pcb2->ptr0[i].pfn, 1, PROT_NONE, (PROT_READ | PROT_WRITE));
            memcpy((void*)(brk_vpn<<PAGESHIFT), (void*)(i<<PAGESHIFT), PAGESIZE);
            TracePrintf(LOG, "mem copy setup successfully!\n");
            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
        }
        pcb1->ptr0[brk_vpn].valid = 0;
    }
    //ptr1[brk_vpn].valid = 0;
    TracePrintf(LOG, "finished copying content of the memory!\n");
    // unsigned long vpn = DOWN_TO_PAGE((void *)pcb2->ptr0 - VMEM_1_BASE) >> PAGESHIFT;
    // unsigned long paddr = (ptr1[vpn].pfn << PAGESHIFT) | (((int)pcb2->ptr0)&PAGEOFFSET);
    // WriteRegister(REG_PTR0,(RCS421RegVal)paddr);
    // WriteRegister(REG_TLB_FLUSH,TLB_FLUSH_0);

    memcpy(pcb1->ctx,ctxp,sizeof(SavedContext));
    pushPCB(pcb2);
    return pcb2->ctx;
}


// 3.5 ContextSwitching
// SavedContext *MySwitchFunc(SavedContext *ctxp, void *p1, void *p2) {
//     PCB *pcb1 = (PCB *)p1;
//     PCB *pcb2 = (PCB *)p2;

//     TracePrintf(LOG, "Context Switch pcb %d to pcb %d\n", pcb1->pid, pcb2->pid);

//     // switch Region 0 page table
//     TracePrintf(INF, "Switch to PT0 %p\n", (void *)pcb2->pt0addr);

//     if (pcb1 == pcb2) {
//         TracePrintf(INF, "Switch to same pcb\n");

//         int pfn = getFreePhysicalFrame();
        
//         if (pfn == -1) return NULL;
        
//         pcb1->ptr0 = (PTE *)malloc(PAGE_TABLE_LEN * sizeof(PTE));
    
//         // convert pcb->ptr0 to physical address
//         int vpn = DOWN_TO_PAGE((void *)pcb1->ptr0 - VMEM_1_BASE) >> PAGESHIFT;
//         int paddr = (ptr1[vpn].pfn << PAGESHIFT) | (((int)pcb1->ptr0)&PAGEOFFSET);
//         pcb1->pt0addr = paddr;
                
//         int i;
//         int tmpVpn = UP_TO_PAGE(kernelBreak - VMEM_1_BASE) >> PAGESHIFT;

//         for (i = 0; i < KERNEL_STACK_PAGES; i++) {
//             int pfn = getFreePhysicalFrame();

//             if (pfn == -1) return NULL;

            
//             if (ptr1[tmpVpn].valid == 1) {
//                 TracePrintf(ERR, "VPN %d should be invalid\n", tmpVpn);
//             }

//             setPTE(&ptr1[tmpVpn], pfn, 1, PROT_NONE, (PROT_READ | PROT_WRITE));
            
//             TracePrintf(TRC, "cpy kernal stack %p into kernelbrk %p\n", (void *)((DOWN_TO_PAGE(KERNEL_STACK_BASE) + PAGESIZE * i)), kernelBreak);
            
//             // copy kernel stack into kernelbrk (into pfn)
//             memcpy((tmpVpn << PAGESHIFT) + VMEM_1_BASE, (void *)((DOWN_TO_PAGE(KERNEL_STACK_BASE) + PAGESIZE * i)), PAGESIZE);

//             setPTE(&pcb1->ptr0[PAGE_TABLE_LEN - 1 - i], pfn, 1, PROT_NONE, (PROT_READ | PROT_WRITE));

//             setPTE(&ptr1[tmpVpn], 0, 0, PROT_NONE, PROT_NONE);
    
//             WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
//         }
        
//         return &(pcb1->ctx);
//     } else {
        
//         WriteRegister(REG_PTR0, (RCS421RegVal)(pcb2->pt0addr));

//         TracePrintf(LOG, "New ptr0 %p\n", (void *)pcb2->pt0addr);

//         ptr1[PT0_VPN].pfn = (pcb2->pt0addr >> PAGESHIFT);

//         WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

//         runningPCB = pcb2;
        
//         return &(runningPCB->ctx);
//     }
// }



PCB *createPCB(int pid) {
    TracePrintf(LOG, "Create New PCB %d\n", pid);

    PCB *pcb = (PCB *)malloc(sizeof(PCB));
    pcb->readyTime = clocktime;
    pcb->pid = pid;
    pcb->parent = NULL;
    pcb->next = NULL;
    pcb->ctx = (SavedContext*)malloc(sizeof(SavedContext));

    if (pid == IDLE_PID) {
        pcb->pt0addr = INIT_PT0_PFN << PAGESHIFT;
        /* address of the idle process*/
        pcb->ptr0 = ptr0;
        pcb->brk = MEM_INVALID_PAGES;
    }
    else if (pid == INIT_PID) {
        TracePrintf(LOG, "Enter else if INIT");
        pcb->brk = MEM_INVALID_PAGES;
        pcb->ptr0 = allocateNewPage();
        // TracePrintf(LOG, "Enter else if: %d INIT2222\n", &pcb->ptr0[1]);
        // pcb->pt0addr = pcb->ptr0 << PAGESHIFT;
        /*mark the page table  entry as valid (int use) for kernel stack*/
        int addr;
        int i;
        for(addr = KERNEL_STACK_BASE; addr < KERNEL_STACK_LIMIT; addr+=PAGESIZE) {
            i = addr>>PAGESHIFT;
            int pfn = getFreePhysicalFrame();
            pcb->ptr0[i].valid = 1;
            pcb->ptr0[i].pfn = pfn;
            pcb->ptr0[i].kprot = PROT_READ | PROT_WRITE;
            pcb->ptr0[i].uprot = PROT_NONE;
    
        }
        TracePrintf(LOG, "Exit else if INIT Process\n");

    }
   
    return pcb;
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