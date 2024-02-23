#include <stdlib.h>
#include <stdio.h>

#include "kernel.h"
#include "call.h"
#include "helper.h"

void *myMalloc(size_t size) {
    TracePrintf(5, "Try to allocte %d memory\n", size);
    void *ptr = malloc(size);
    if (ptr == NULL) {
        TracePrintf(0, "Malloc Failed\n");
    }
    
    return ptr;
}

// Initialize the interrupt vector table entries for each type of interrupt, exception, or trap, by making
// them point to the correct handler functions in your kernel.
void initInterruptVectorTable(TrapHandlerPtr *InterruptVectorTable) {
    TracePrintf(5, "start init Interrupt Vector Table\n");

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
    TracePrintf(5, "start init Free Physical Page\n");

    int totalPageNum = DOWN_TO_PAGE(pmem_size) >> PAGESHIFT;
    // int orig_brk_pg_num = UP_TO_PAGE(orig_brk) >> PAGESHIFT;

    PhysicalPages->isFree = (int *)myMalloc(totalPageNum * sizeof(int));
    PhysicalPages->freePageNum = 0;

    int pfn = 0;
    for (pfn = 0; pfn < totalPageNum; pfn++) {
        // The page for kernel stack and kernel heap are not free (According to Fig 4, not very sure about this part.)
        if (pfn >= DOWN_TO_PAGE(KERNEL_STACK_BASE) && pfn < DOWN_TO_PAGE(orig_brk)) PhysicalPages->isFree[pfn] = 0;
        else {
            PhysicalPages->freePageNum++;
            PhysicalPages->isFree[pfn] = 1;
        }
    }
}

void initPageTable(PageTable *pageTable, void *orig_brk) {
    TracePrintf(5, "start init Page Table\n");

    int orig_brk_pg_num = UP_TO_PAGE(orig_brk) >> PAGESHIFT;
    pageTable->region0 = (struct pte *)myMalloc(PAGE_TABLE_SIZE);
    pageTable->region1 = (struct pte *)myMalloc(PAGE_TABLE_SIZE);

    int i = 0;
    // a page table entry should be built so that the new vpn = pfn

    // Kernel stack
    for (i = 0; i < KERNEL_STACK_PAGES; i++) {
        setPTE(&pageTable->region0[PAGE_TABLE_LEN - 1 - i], PAGE_TABLE_LEN - 1 - i, 1, PROT_NONE, (PROT_READ | PROT_WRITE));
    }

    for (i = 0; i < orig_brk_pg_num; i++) {
        // Kernel data/bss/heap
        setPTE(&pageTable->region1[i], i + PAGE_TABLE_LEN, 1, PROT_NONE, (PROT_READ | PROT_WRITE));
        
        // Kernel text
        if (i * PAGESIZE + VMEM_1_BASE < &_etext) pageTable->region1[i].kprot = (PROT_READ | PROT_EXEC);
    }

    //Setup REG_PTR0 and REG_PTR1
	WriteRegister(REG_PTR0, (RCS421RegVal) pageTable->region0);
	WriteRegister(REG_PTR1, (RCS421RegVal) pageTable->region1);
}