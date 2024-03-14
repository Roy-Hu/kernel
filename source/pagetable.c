// #include "pagetable.h"
#include "pcb.h"
#include "global.h"
#include <stddef.h>

int totalPhysicalFrameNum;

PhysicalFrame physicalFrames;

PTE *ptr0, ptr1[PAGE_TABLE_LEN];
page_table0 *head_ptr0;




void *kernelBreak;

void freePhysicalFrame(int pfn) {
    if (physicalFrames.isFree[pfn] == 0) physicalFrames.freePFN++;
    physicalFrames.isFree[pfn] = 1;
}

/*return the starting address of the new ptr0*/
PTE* allocateNewPage() {
    struct page_table0* curr = head_ptr0;
    TracePrintf(LOG, "Enter else if INIT 2\n");
   

    while (curr != NULL) {
        /*empty space found, do not allocate new space*/
        if (curr->is_full == 0) {
            curr->is_full = 1;
            int pfn = getFreePhysicalFrame();
            int vpn = ((unsigned long)(curr->start_addr) - VMEM_1_BASE) >> PAGESHIFT;
            /*set the kernel pagetable*/
            setPTE(&ptr1[vpn], pfn, 1, PROT_NONE, (PROT_READ | PROT_WRITE));
            TracePrintf(LOG, "Return from ALlocate pte\n");
            return curr->start_addr;
        }
        if (curr->nextPage != NULL)
            curr = curr->nextPage;
        else break;  
    }
    TracePrintf(LOG, "Enter else if INIT\n");

     /*no empty space, allocate new pt0*/
    struct page_table0* new_pt0 = malloc(sizeof(struct page_table0));

    // TracePrintf(TRC, "currptr0%p\n",curr->start_addr);

    /*new virtual address of the r0 pt*/
    new_pt0->start_addr =(PTE*)(curr->start_addr - PAGESIZE);
    TracePrintf(LOG, "new address succeed\n");
    new_pt0->is_full = 1;
    new_pt0->nextPage = NULL;

    int pfn = getFreePhysicalFrame();
   
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
    int i = 0;
    int count = 0;
    for (; i < PAGE_TABLE_LEN; ++i) {
        if (runningPCB->ptr0[i].valid == 1) {
            count++;
        }
    }
    int freepage = physicalFrames.freePFN;
    return (freepage>=count);
}

/**
 * return value: user stack boundary address
*/
int user_stack_vpn() {
    int i = UP_TO_PAGE(USER_STACK_LIMIT) >> PAGESHIFT;
    int brk = UP_TO_PAGE(runningPCB->brk) >> PAGESHIFT;
    for (; i > brk; --i) {
        if (runningPCB->ptr0[i].valid != 1) break;
    }
    return i;
}