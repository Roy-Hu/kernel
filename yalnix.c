#include <trap.h>
#include <call.h>
#include <kernel.h>

#include <stdio.h>

TrapHandlerPtr interruptVectorTable[TRAP_VECTOR_SIZE];

PhysicalPage physicalPages;

PageTable pageTable;
// 3.3 Kernel Boot Entry Point

// Kernel Start procedure should initialize the operating system kernel and then return,
// starting the first process executing

// ExceptionInfo: records the state of the machine at boot time, and any changes made in the values
// in this ExceptionInfo control how and where the machine will execute when KernelStart
// returns

// pmem_size: gives the total size of the physical memory of the machine you are
// running on, in bytes

// orig_brk:  gives the initial value of the kernel’s “break.” That is, this address is the
// first address that is not part of the kernel’s initial heap.

// cmd_args:  is a vector of strings (in the same format as argv for normal Unix main
// programs), containing a pointer to each argument from the boot command line (what you typed at
// your Linux terminal) to start the machine and thus the kernel. The cmd_args vector is terminated
// by a NULLpointer.

void KernelStart(ExceptionInfo *info, unsigned int pmem_size, 
	void *orig_brk, char **cmd_args) {
		
	TracePrintf(5, "Kernel Start, Total physical memory %d\n", pmem_size);
	
	// Initialize the interrupt vector table and REG_VECTOR_BASE privileged machine register
	initInterruptVectorTable(interruptVectorTable);

	// Initialize the free physical page frames
	initFreePhysicalPage(&physicalPages, pmem_size, orig_brk);

	// Initialize the page table
	initPageTable(&pageTable, orig_brk);

	WriteRegister(REG_VM_ENABLE, 1);
}

// 3.4.2 Kernel Memory Management
int SetKernelBrk(void *addr) {
}

// 3.5 ContextSwitching
SavedContext *MySwitchFunc(SavedContext *ctxp, void *p1, void *p2) {
}
