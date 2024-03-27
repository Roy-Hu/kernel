#include "pcb.h"
#include "global.h"
#include <stddef.h>

int totalPhysicalFrameNum;

PhysicalFrame physicalFrames;

PTE *ptr0, ptr1[PAGE_TABLE_LEN];
PageTable0 *head_ptr0;

void *kernelBreak;

void freePhysicalFrame(int pfn) {
    if (physicalFrames.isFree[pfn] == 0) physicalFrames.freePFN++;
    physicalFrames.isFree[pfn] = 1;
}

/*return the starting address of the new ptr0*/
PTE* allocateNewPage() {
    PageTable0 *curr = head_ptr0;

    // find the last page_table0
    while (curr != NULL) {
        /*empty space found, do not allocate new space*/
        // it seems is_full will never be 0?
        if (curr->is_full == 0) {
            curr->is_full = 1;
            int pfn = getFreePhysicalFrame();
            
            int vpn = ((unsigned long)(curr->start_addr) - VMEM_1_BASE) >> PAGESHIFT;
            /*set the kernel pagetable*/
            setPTE(&ptr1[vpn], pfn, 1, PROT_NONE, (PROT_READ | PROT_WRITE));
            TracePrintf(TRC, "Return from Allocate pte\n");

            return curr->start_addr;
        }
 
        if (curr->nextPage != NULL) curr = curr->nextPage;
        else break;  
    }

     /*no empty space, allocate new pt0*/
    PageTable0 *new_pt0 = malloc(sizeof(PageTable0));

    // TracePrintf(TRC, "currptr0%p\n",curr->start_addr);

    /*new virtual address of the ptr0*/
    new_pt0->start_addr =(PTE *)(curr->start_addr - PAGESIZE);
    // TracePrintf(LOG, "new address succeed\n");
    new_pt0->is_full = 1;
    new_pt0->nextPage = NULL;

    int pfn = getFreePhysicalFrame();
    if (pfn == -1) {
        return NULL;
    } 

    int vpn = ((unsigned long)(new_pt0->start_addr) - VMEM_1_BASE) >> PAGESHIFT;
    /*set the kernel pagetable*/
    setPTE(&ptr1[vpn], pfn, 1, PROT_NONE, (PROT_READ | PROT_WRITE));

    curr->nextPage = new_pt0;

    return new_pt0->start_addr;
}

int getFreePhysicalFrame() {
    int pfn;
    for (pfn = 0; pfn < physicalFrames.totalPFN; pfn++) {
        if (physicalFrames.isFree[pfn] == 1) {
            physicalFrames.isFree[pfn] = 0;
            physicalFrames.freePFN--;

            // TracePrintf(TRC, "Get free physical frame %d\n", pfn);
            return pfn;
        }
    }
    
    TracePrintf(ERR, "No free physical frame\n");

    return -1;
}

void setPTE(PTE *entry, int pfn, int valid, int uprot, int kprot) {
    entry->pfn = pfn;
    entry->valid = valid;
    entry->uprot = uprot;
    entry->kprot = kprot;
}

/**
 * Functions to check for enough pages allocated for Child Process
 * Return value: 0: not enough physical pages, 1: enough free physical pages
 * 
*/
int check_enough_pages_fork() {
    int i;
    int count = 0;

    for (i = 0; i < PAGE_TABLE_LEN; ++i) {
        if (runningPCB->ptr0[i].valid == 1) count++;
    }

    int freepage = physicalFrames.freePFN;

    return (freepage >= count);
}

/**
 * Find the boundary of the current user stack
 * starting from USER_STACK_LIMIT
 * return value: user stack boundary address (virtual)
*/
int user_stack_vpn() {
    int i = (DOWN_TO_PAGE(USER_STACK_LIMIT) >> PAGESHIFT) - 1;

    TracePrintf(LOG, "User stack limit: %d\n", i);
    int brk = UP_TO_PAGE(runningPCB->brk) >> PAGESHIFT;

    for (; i > brk; --i) {
        if (runningPCB->ptr0[i].valid != 1) break;
    }

    return i;
}