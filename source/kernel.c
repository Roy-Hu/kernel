#include <stdlib.h>
#include <stdio.h>

#include "kernel.h"
#include "call.h"
#include "helper.h"

// Initialize the interrupt vector table entries for each type of interrupt, exception, or trap, by making
// them point to the correct handler functions in your kernel.
void initInterruptVectorTable(TrapHandlerPtr *InterruptVectorTable) {
    TracePrintf(LOG, "start init Interrupt Vector Table\n");

	InterruptVectorTable[TRAP_KERNEL] = &TrapKernelHandler;
	InterruptVectorTable[TRAP_CLOCK] = &TrapClockHandler;
	InterruptVectorTable[TRAP_ILLEGAL] = &TrapIllegalHandler;
	InterruptVectorTable[TRAP_MEMORY] = &TrapMemoryHandler;
	InterruptVectorTable[TRAP_MATH] = &TrapMathHandler;
	InterruptVectorTable[TRAP_TTY_RECEIVE] = &TrapTtyReceiveHandler;
	InterruptVectorTable[TRAP_TTY_TRANSMIT] = &TrapTtyTransmitHandler;

    // Set the unused entries to NULL
    int i;
    for (i = TRAP_TTY_TRANSMIT + 1; i < TRAP_VECTOR_SIZE; i++) InterruptVectorTable[i] = NULL;

    // Initialize the REG_VECTOR_BASE privileged machine register to point to your interrupt vector table.
	WriteRegister(REG_VECTOR_BASE, (RCS421RegVal) InterruptVectorTable);
}

// Build a structure to keep track of what page frames in physical memory are free
void initFreePhysicalPage(PhysicalPage *PhysicalPages, unsigned int pmem_size, void *orig_brk) {
    TracePrintf(LOG, "start init Free Physical Page\n");

    int totalPageNum = DOWN_TO_PAGE(pmem_size) >> PAGESHIFT;

    TracePrintf(TRC, "Original break %p\n", orig_brk);

    // Allocate hysical space for the isFree array
    PhysicalPages->isFree = (int *)orig_brk;
    orig_brk += totalPageNum * sizeof(int);

    TracePrintf(TRC, "Original break After allocate phisical Pages %p\n", orig_brk);

    PhysicalPages->freePageNum = 0;

    int pfn = 0;
    for (pfn = 0; pfn < totalPageNum; pfn++) {
        // The page for kernel stack and kernel heap are not free (According to Fig 4, not very sure about this part.)
        if (pfn >= DOWN_TO_PAGE(KERNEL_STACK_BASE) && pfn < UP_TO_PAGE(orig_brk)) PhysicalPages->isFree[pfn] = 0;
        else {
            PhysicalPages->freePageNum++;
            PhysicalPages->isFree[pfn] = 1;
        }
    }
}

void initPageTable(PageTable *region0, PageTable *region1, void *orig_brk) {
    TracePrintf(LOG, "start init Page Table\n");

    TracePrintf(TRC, "Original break %p\n", orig_brk);

    // Piazza @84
    // For your initial Region 0 page table, it is best to take some physical page 
    // and put that page table in the first half of that physical page. You then know 
    // it's physical address for writing into REG_PTR0.  And you can set up a PTE for 
    // it so that it appears to be at any virtual address you want. 

    region0 = (PageTable *)orig_brk;
    orig_brk += PAGE_TABLE_SIZE;

    TracePrintf(TRC, "Original break After allocate Region 0 Page Table %p\n", orig_brk);

    int orig_brk_pg_num = UP_TO_PAGE(orig_brk) >> PAGESHIFT;

    int i = 0;
    // a page table entry should be built so that the new vpn = pfn

    // Kernel stack
    for (i = 0; i < KERNEL_STACK_PAGES; i++) {
        setPTE(&region0[PAGE_TABLE_LEN - 1 - i], PAGE_TABLE_LEN - 1 - i, 1, PROT_NONE, (PROT_READ | PROT_WRITE));
    }

    for (i = 0; i < orig_brk_pg_num; i++) {
        // Kernel data/bss/heap
        setPTE(&region1[i], i + PAGE_TABLE_LEN, 1, PROT_NONE, (PROT_READ | PROT_WRITE));
        
        // Kernel text
        if (i * PAGESIZE + VMEM_1_BASE < &_etext) region1[i].kprot = (PROT_READ | PROT_EXEC);
    }

    //Setup REG_PTR0 and REG_PTR1
	WriteRegister(REG_PTR0, (RCS421RegVal) region0);
	WriteRegister(REG_PTR1, (RCS421RegVal) region1);
}