#include <trap.h>
#include <call.h>
#include <kernel.h>

#include <stdio.h>

TrapHandlerPtr interruptVectorTable[TRAP_VECTOR_SIZE];

PhysicalPage physicalPages;

PageTable *region0, region1[PAGE_TABLE_LEN];

int vmEnable = 0;

void *kernelBreak;
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
	
	void *newBrk;
	TracePrintf(LOG, "Kernel Start, Total physical memory %d, current bread %p\n", pmem_size, orig_brk);
	
	// Initialize the interrupt vector table and REG_VECTOR_BASE privileged machine register
	initInterruptVectorTable(interruptVectorTable);

	SetKernelBrk(orig_brk);

	// Initialize the free physical page frames
	newBrk = initFreePhysicalPage(&physicalPages, pmem_size, kernelBreak);
	SetKernelBrk(newBrk);

	// Initialize the page table
	newBrk = initPageTable(region0, region1, kernelBreak);
	SetKernelBrk(newBrk);

	// Enable virtual memory
	WriteRegister(REG_VM_ENABLE, 1);
	

	vmEnable = 1;

	TracePrintf(LOG, "Enable vitrual memory, current kernel break %p\n", kernelBreak);

	Halt();
}

// 3.4.2 Kernel Memory Management
int SetKernelBrk(void *addr) {
	if (!vmEnable) {
		if ((unsigned long)addr > VMEM_1_LIMIT) return -1;

		kernelBreak = addr;

		return 0;
	} else {
		
	}

	return -1;
}

// 3.5 ContextSwitching
SavedContext *MySwitchFunc(SavedContext *ctxp, void *p1, void *p2) {
}
