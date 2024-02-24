#include <stdlib.h>
#include <stdio.h>

#include "init.h"
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
void *initFreePhysicalFrame(PhysicalFrame *PhysicalFrames, int totalPhysicalFrameNum, void *brk) {
    TracePrintf(LOG, "start init Free Physical Page\n");

    // Allocate hysical space for the isFree array
    PhysicalFrames->isFree = (int *)brk;
    int allocatedMemory = totalPhysicalFrameNum * sizeof(int);

    int kernelStackBasePgNum = DOWN_TO_PAGE(KERNEL_STACK_BASE) >> PAGESHIFT;

    void* newBrk = brk + allocatedMemory;
    int newBrkPgNum = UP_TO_PAGE(newBrk) >> PAGESHIFT;

    PhysicalFrames->freePageNum = 0;
    
    int pfn;
    for (pfn = 0; pfn < totalPhysicalFrameNum; pfn++) {
        // The page for kernel stack and kernel heap are not free (According to Fig 4, not very sure about this part.)
        // The page (at VMEM_1_LIMIT - PAGESIZE) for region0 page table is not free
        if ((pfn >= kernelStackBasePgNum && pfn < newBrkPgNum) || pfn == PAGE_TABLE_LEN * 2 - 1) PhysicalFrames->isFree[pfn] = 0;
        else {
            PhysicalFrames->freePageNum++;
            PhysicalFrames->isFree[pfn] = 1;
        }
    }

    TracePrintf(TRC, "free Physical Page (Total - kernel - region0 table) %d\n", PhysicalFrames->freePageNum);

    return newBrk;
}

void initPageTable(PageTable *region0, PageTable *region1, void *brk) {
    TracePrintf(LOG, "start init Page Table\n");

    // Piazza @84
    // For your initial Region 0 page table, it is best to take some physical page 
    // and put that page table in the first half of that physical page. You then know 
    // it's physical address for writing into REG_PTR0.  And you can set up a PTE for 
    // it so that it appears to be at any virtual address you want. An easy place to 
    // make it appear (not the only choice) is at the top virtual page address in Region 1 
    // (at virtual address VMEM_1_LIMIT - PAGESIZE) by using the top PTE in the Region 1 
    // page table you are setting up.

    region0 = (PageTable *)(VMEM_1_LIMIT - PAGESIZE);;

    // set the corresponding Physical page to invalid
    int region0FramePg = DOWN_TO_PAGE(VMEM_1_LIMIT - PAGESIZE) >> PAGESHIFT;

    setPTE(&region1[PAGE_TABLE_LEN - 1], 2*PAGE_TABLE_LEN - 1, 1, PROT_NONE, (PROT_READ | PROT_WRITE));
    // Becareful about the DOWN_TO_PAGE and UP_TO_PAGE usage
    
    int i = 0;
    // a page table entry should be built so that the new vpn = pfn

    // Kernel stack
    for (i = 0; i < KERNEL_STACK_PAGES; i++) {
        setPTE(&region0[PAGE_TABLE_LEN - 1 - i], PAGE_TABLE_LEN - 1 - i, 1, PROT_NONE, (PROT_READ | PROT_WRITE));
    }

    for (i = 0; i < UP_TO_PAGE(brk - VMEM_1_BASE) >> PAGESHIFT; i++) {
        // Kernel data/bss/heap
        setPTE(&region1[i], i + PAGE_TABLE_LEN, 1, PROT_NONE, (PROT_READ | PROT_WRITE));
        
        // Kernel text
        if (i * PAGESIZE + VMEM_1_BASE < &_etext) region1[i].kprot = (PROT_READ | PROT_EXEC);
    }

    //Setup REG_PTR0 and REG_PTR1
	WriteRegister(REG_PTR0, (RCS421RegVal) region0);
	WriteRegister(REG_PTR1, (RCS421RegVal) region1);
}