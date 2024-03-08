#include <stddef.h>
#include <stdlib.h>

#include "pcb.h"
#include "global.h"

PCB *runningPCB, *waitingPCB, *readyPCB, *idlePCB, *initPCB;

PCB *readyPCBTail, *waitingPCBTail;

// 3.5 ContextSwitching
SavedContext *MySwitchFunc(SavedContext *ctxp, void *p1, void *p2) {
    PCB *pcb1 = (PCB *)p1;
    PCB *pcb2 = (PCB *)p2;

    TracePrintf(LOG, "Context Switch pcb %d to pcb %d\n", pcb1->pid, pcb2->pid);

    // switch Region 0 page table
    TracePrintf(INF, "Switch to PT0 %p\n", (void *)pcb2->pt0addr);

    if (pcb1 == pcb2) {
        TracePrintf(INF, "Switch to same pcb\n");

        int pfn = getFreePhysicalFrame();
        
        if (pfn == -1) return NULL;
        
        pcb1->ptr0 = (PTE *)malloc(PAGE_TABLE_LEN * sizeof(PTE));
    
        // convert pcb->ptr0 to physical address
        int vpn = DOWN_TO_PAGE((void *)pcb1->ptr0 - VMEM_1_BASE) >> PAGESHIFT;
        int paddr = (ptr1[vpn].pfn << PAGESHIFT) | (((int)pcb1->ptr0)&PAGEOFFSET);
        pcb1->pt0addr = paddr;
                
        int i;
        int tmpVpn = UP_TO_PAGE(kernelBreak - VMEM_1_BASE) >> PAGESHIFT;

        for (i = 0; i < KERNEL_STACK_PAGES; i++) {
            int pfn = getFreePhysicalFrame();

            if (pfn == -1) return NULL;

            
            if (ptr1[tmpVpn].valid == 1) {
                TracePrintf(ERR, "VPN %d should be invalid\n", tmpVpn);
            }

            setPTE(&ptr1[tmpVpn], pfn, 1, PROT_NONE, (PROT_READ | PROT_WRITE));
            
            TracePrintf(TRC, "cpy kernal stack %p into kernelbrk %p\n", (void *)((DOWN_TO_PAGE(KERNEL_STACK_BASE) + PAGESIZE * i)), kernelBreak);
            
            // copy kernel stack into kernelbrk (into pfn)
            memcpy((tmpVpn << PAGESHIFT) + VMEM_1_BASE, (void *)((DOWN_TO_PAGE(KERNEL_STACK_BASE) + PAGESIZE * i)), PAGESIZE);

            setPTE(&pcb1->ptr0[PAGE_TABLE_LEN - 1 - i], pfn, 1, PROT_NONE, (PROT_READ | PROT_WRITE));

            setPTE(&ptr1[tmpVpn], 0, 0, PROT_NONE, PROT_NONE);
    
            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
        }
        
        return &(pcb1->ctx);
    } else {
        
        WriteRegister(REG_PTR0, (RCS421RegVal)(pcb2->pt0addr));

        TracePrintf(LOG, "New ptr0 %p\n", (void *)pcb2->pt0addr);

        ptr1[PT0_VPN].pfn = (pcb2->pt0addr >> PAGESHIFT);

        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

        runningPCB = pcb2;
        
        return &(runningPCB->ctx);
    }
}

PCB *createPCB(int pid) {
    TracePrintf(LOG, "Create New PCB %d\n", pid);

    PCB *pcb = (PCB *)malloc(sizeof(PCB));
    
    pcb->pid = pid;
    pcb->next = NULL;

    if (pid == INIT_PID) {
        pcb->pt0addr = INIT_PT0_PFN << PAGESHIFT;
        pcb->ptr0 = ptr0;
    } else {
        ContextSwitch(MySwitchFunc, &pcb->ctx, (void *)pcb, (void *)pcb);
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