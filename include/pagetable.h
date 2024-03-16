#ifndef PAGETABLE_H_
#define PAGETABLE_H_

#include <comp421/hardware.h>

#define INIT_PT0_PFN (2 * PAGE_TABLE_LEN - 1)
#define PT0_VPN (PAGE_TABLE_LEN - 1)

typedef struct pte PTE;

// struct page_table0*;
/*keep track of the head of pagetable*/

extern PTE *ptr0, ptr1[PAGE_TABLE_LEN];

/*current ptr0*/
// extern PTE *ptr0;

extern void *kernelBreak;

/*keeping track of the base address of the pagetable and its availability*/
typedef struct page_table0 {
    int is_full;

    // start virtual address of the page table 0
    // The start_addr will be VMEM_1_LIMIT - PAGESIZE, for the following page tables, 
    // it will be the start_addr - PAGESIZE
    void* start_addr;
    
    struct page_table0 *nextPage;
} PageTable0;

typedef struct physicalFrame {
    int totalPFN;
    int freePFN;
    int *isFree;
} PhysicalFrame;

extern int totalPhysicalFrameNum;

extern PhysicalFrame physicalFrames;

extern PageTable0 *head_ptr0;

void freePhysicalFrame(int pfn);

int getFreePhysicalFrame();

int check_enough_pages_fork();

int user_stack_vpn();

PTE* allocateNewPage();

void setPTE(struct pte *entry, int pfn, int valid, int uprot, int kprot);

#endif