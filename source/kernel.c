#include <stdlib.h>
#include <stdio.h>
#include "kernel.h"
#include "call.h"

void *myMalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "malloc failed.\n");
        Exit(ERROR);
    }
    
    return ptr;
}

// Initialize the interrupt vector table entries for each type of interrupt, exception, or trap, by making
// them point to the correct handler functions in your kernel.
void initInterruptVectorTable(TrapHandlerPtr *InterruptVectorTable) {

	InterruptVectorTable = myMalloc(TRAP_VECTOR_SIZE * sizeof(TrapHandlerPtr));
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
    int totalPageNum = DOWN_TO_PAGE(pmem_size) >> PAGESHIFT;
    int orig_brk_pg_num = UP_TO_PAGE(orig_brk) >> PAGESHIFT;

    PhysicalPages->isFree = myMalloc(totalPageNum * sizeof(bool));
    PhysicalPages->freePageNum = 0;

    int i = 0;
    for (i = 0; i < totalPageNum; i++) {
        // The page for kernel stack and kernel heap are not free (According to Fig 4, not very sure about this part.)
        if (i >= VMEM_0_SIZE - KERNEL_STACK_SIZE && i < VMEM_1_BASE + orig_brk) PhysicalPages->isFree[i] = false;
        else {
            PhysicalPages->freePageNum++;
            PhysicalPages->isFree[i] = true;
        }
    }
}
