#include <comp421/yalnix.h>
#include <trap.h>

#include <stdio.h>

typedef struct pcb {
} PCB;

// 3.3 KernelBootEntryPoint
void KernelStart(ExceptionInfo *info, unsigned int pmem_size, 
	void *orig_brk, char **cmd_args) {

}

// 3.4.2 Kernel Memory Management
int SetKernelBrk(void *addr) {
}

// 3.5 ContextSwitching
SavedContext *MySwitchFunc(SavedContext *ctxp, void *p1, void *p2) {
}
