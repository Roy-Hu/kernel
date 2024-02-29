#include "trap.h"
#include "call.h"
#include "init.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <comp421/loadinfo.h>

int totalPhysicalFrameNum;

TrapHandlerPtr interruptVectorTable[TRAP_VECTOR_SIZE];

PhysicalFrame physicalFrames;

PTE *ptr0, ptr1[PAGE_TABLE_LEN];

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

	totalPhysicalFrameNum = DOWN_TO_PAGE(pmem_size) >> PAGESHIFT;

	TracePrintf(LOG, "Kernel Start, Total Physical Memory %d Btyes/%d Pages\n", pmem_size, totalPhysicalFrameNum);

	SetKernelBrk(orig_brk);

	// Initialize the interrupt vector table and REG_VECTOR_BASE privileged machine register
	initInterruptVectorTable(interruptVectorTable);

	// Initialize the free physical page frames
	newBrk = initFreePhysicalFrame(&physicalFrames, totalPhysicalFrameNum, kernelBreak);
	SetKernelBrk(newBrk);

	// Initialize the page table
	ptr0 = initPageTable(ptr1, kernelBreak);
    
    TracePrintf(TRC, "ptr0 %p, ptr1 %p\n", ptr0, ptr1);

	// Enable virtual memory
	WriteRegister(REG_VM_ENABLE, 1);
	
	vmEnable = 1;

	TracePrintf(LOG, "Enable vitrual memory\n");

    LoadProgram("idle", cmd_args, info);
    
    TracePrintf(LOG, "KernelStart Success\n");
}

// 3.4.2 Kernel Memory Management
int SetKernelBrk(void *addr) {
	if (!vmEnable) {
		if ((unsigned long)addr > VMEM_1_LIMIT) return -1;

		kernelBreak = addr;

		TracePrintf(TRC, "New kernel break %p, page num %d\n", kernelBreak, DOWN_TO_PAGE(kernelBreak) >> PAGESHIFT);

		return 0;
	} else {
		TracePrintf(TRC, "New kernel break Vitrual Memory Enable\n");
	}

	return -1;
}

// 3.5 ContextSwitching
SavedContext *MySwitchFunc(SavedContext *ctxp, void *p1, void *p2) {
}

/*
 *  Load a program into the current process's address space.  The
 *  program comes from the Unix file identified by "name", and its
 *  arguments come from the array at "args", which is in standard
 *  argv format.
 *
 *  Returns:
 *      0 on success
 *     -1 on any error for which the current process is still runnable
 *     -2 on any error for which the current process is no longer runnable
 *
 *  This function, after a series of initial checks, deletes the
 *  contents of Region 0, thus making the current process no longer
 *  runnable.  Before this point, it is possible to return ERROR
 *  to an Exec() call that has called LoadProgram, and this function
 *  returns -1 for errors up to this point.  After this point, the
 *  contents of Region 0 no longer exist, so the calling user process
 *  is no longer runnable, and this function returns -2 for errors
 *  in this case.
 */

int LoadProgram(char *name, char **args, ExceptionInfo *info) {
    int fd;
    int status;
    struct loadinfo li;
    char *cp;
    char *cp2;
    char **cpp;
    char *argbuf;
    int i;
    unsigned long argcount;
    int size;
    int text_npg;
    int data_bss_npg;
    int stack_npg;

    TracePrintf(LOG, "LoadProgram '%s', args %p\n", name, args);

    if ((fd = open(name, O_RDONLY)) < 0) {
        TracePrintf(ERR, "LoadProgram: can't open file '%s'\n", name);
        return (-1);
    }

    status = LoadInfo(fd, &li);
    TracePrintf(INF, "LoadProgram: LoadInfo status %d\n", status);
    switch (status) {
	case LI_SUCCESS:
	    break;
	case LI_FORMAT_ERROR:
	    TracePrintf(ERR,
		"LoadProgram: '%s' not in Yalnix format\n", name);
	    close(fd);
	    return (-1);
	case LI_OTHER_ERROR:
	    TracePrintf(ERR, "LoadProgram: '%s' other error\n", name);
	    close(fd);
	    return (-1);
	default:
	    TracePrintf(ERR, "LoadProgram: '%s' unknown error\n", name);
	    close(fd);
	    return (-1);
    }

    TracePrintf(INF, "text_size 0x%lx, data_size 0x%lx, bss_size 0x%lx\n",
	    li.text_size, li.data_size, li.bss_size);
    TracePrintf(INF, "entry 0x%lx\n", li.entry);

    /*
     *  Figure out how many bytes are needed to hold the arguments on
     *  the new stack that we are building.  Also count the number of
     *  arguments, to become the argc that the new "main" gets called with.
     */
    size = 0;
    for (i = 0; args[i] != NULL; i++) size += strlen(args[i]) + 1;

    argcount = i;
    TracePrintf(INF, "LoadProgram: size %d, argcount %d\n", size, argcount);

    /*
     *  Now save the arguments in a separate buffer in Region 1, since
     *  we are about to delete all of Region 0.
     */
    cp = argbuf = (char *)malloc(size);
    for (i = 0; args[i] != NULL; i++) {
        strcpy(cp, args[i]);
        cp += strlen(cp) + 1;
    }

    /*
     *  The arguments will get copied starting at "cp" as set below,
     *  and the argv pointers to the arguments (and the argc value)
     *  will get built starting at "cpp" as set below.  The value for
     *  "cpp" is computed by subtracting off space for the number of
     *  arguments plus 4 (for the argc value, a 0 (AT_NULL) to
     *  terminate the auxiliary vector, a NULL pointer terminating
     *  the argv pointers, and a NULL pointer terminating the envp
     *  pointers) times the size of each (sizeof(void *)).  The
     *  value must also be aligned down to a multiple of 8 boundary.
     */
    cp = ((char *)USER_STACK_LIMIT) - size;
    cpp = (char **)((unsigned long)cp & (-1 << 4));	/* align cpp */
    cpp = (char **)((unsigned long)cpp - ((argcount + 4) * sizeof(void *)));

    text_npg = li.text_size >> PAGESHIFT;
    data_bss_npg = UP_TO_PAGE(li.data_size + li.bss_size) >> PAGESHIFT;
    stack_npg = (USER_STACK_LIMIT - DOWN_TO_PAGE(cpp)) >> PAGESHIFT;

    TracePrintf(INF, "LoadProgram: text_npg %d, data_bss_npg %d, stack_npg %d\n",
	    text_npg, data_bss_npg, stack_npg);

    /*
     *  Make sure we have enough *virtual* memory to fit everything within
     *  the size of a page table, including leaving at least one page
     *  between the heap and the user stack
     */
    if (MEM_INVALID_PAGES + text_npg + data_bss_npg + 1 + stack_npg +
	    1 + KERNEL_STACK_PAGES > PAGE_TABLE_LEN) {
        
        TracePrintf(ERR,
            "LoadProgram: program '%s' size too large for VIRTUAL memory\n",
            name);

        free(argbuf);
        close(fd);
        return (-1);
    }

    /*
     *  And make sure there will be enough *physical* memory to
     *  load the new program.
     */
    // >>>> The new program will require text_npg pages of text,
    // >>>> data_bss_npg pages of data/bss, and stack_npg pages of
    // >>>> stack.  In checking that there is enough free physical
    // >>>> memory for this, be sure to allow for the physical memory
    // >>>> pages already allocated to this process that will be
    // >>>> freed below before we allocate the needed pages for
    // >>>> the new program being loaded.
    if (text_npg + data_bss_npg + stack_npg > physicalFrames.freePFN) {
        TracePrintf(ERR,
            "LoadProgram: program '%s' size too large for PHYSICAL memory\n",
            name);
        free(argbuf);
        close(fd);
        return (-1);
    }

    // >>>> Initialize sp for the current process to (void *)cpp.
    // >>>> The value of cpp was initialized above.

    info->sp = (void *)cpp;

    /*
     *  Free all the old physical memory belonging to this process,
     *  but be sure to leave the kernel stack for this process (which
     *  is also in Region 0) alone.
     */
    // >>>> Loop over all PTEs for the current process's Region 0,
    // >>>> except for those corresponding to the kernel stack (between
    // >>>> address KERNEL_STACK_BASE and KERNEL_STACK_LIMIT).  For
    // >>>> any of these PTEs that are valid, free the physical memory
    // >>>> memory page indicated by that PTE's pfn field.  Set all
    // >>>> of these PTEs to be no longer valid.

    TracePrintf(TRC, "LoadProgram: free old KERNEL_STACK_PAGES 0 ~ %d\n", PAGE_TABLE_LEN - KERNEL_STACK_PAGES);
    for (i = 0; i < PAGE_TABLE_LEN - KERNEL_STACK_PAGES; i++) {
        if (ptr0[i].valid == 1) {
            freePhysicalFrame(&physicalFrames, ptr0[i].pfn);
            ptr0[i].valid = 0;
        }
    }

    /*
     *  Fill in the page table with the right number of text,
     *  data+bss, and stack pages.  We set all the text pages
     *  here to be read/write, just like the data+bss and
     *  stack pages, so that we can read the text into them
     *  from the file.  We then change them read/execute.
     */

    // >>>> Leave the first MEM_INVALID_PAGES number of PTEs in the
    // >>>> Region 0 page table unused (and thus invalid)

    for (i = 0; i < MEM_INVALID_PAGES; i++) ptr0[i].valid = 0;

    /* First, the text pages */
    // >>>> For the next text_npg number of PTEs in the Region 0
    // >>>> page table, initialize each PTE:
    // >>>>     valid = 1
    // >>>>     kprot = PROT_READ | PROT_WRITE
    // >>>>     uprot = PROT_READ | PROT_EXEC
    // >>>>     pfn   = a new page of physical memory
    int pfn;

    TracePrintf(TRC, "LoadProgram: set text pages %d ~ %d\n", MEM_INVALID_PAGES, MEM_INVALID_PAGES + text_npg);
    for (i = MEM_INVALID_PAGES; i < MEM_INVALID_PAGES + text_npg; i++) {
        pfn = getFreePhysicalFrame(&physicalFrames);
        if (pfn == -1) {
            TracePrintf(ERR, "No free physical frame\n");
            free(argbuf);
            close(fd);
            return (-1);
        }

        setPTE(&ptr0[i], pfn, 1, PROT_READ | PROT_EXEC, PROT_READ | PROT_WRITE);
    }

    // /* Then the data and bss pages */
    // >>>> For the next data_bss_npg number of PTEs in the Region 0
    // >>>> page table, initialize each PTE:
    // >>>>     valid = 1
    // >>>>     kprot = PROT_READ | PROT_WRITE
    // >>>>     uprot = PROT_READ | PROT_WRITE
    // >>>>     pfn   = a new page of physical memory

    TracePrintf(TRC, "LoadProgram: set data and bss pages %d ~ %d\n", MEM_INVALID_PAGES + text_npg, MEM_INVALID_PAGES + text_npg + data_bss_npg);
    for (i = MEM_INVALID_PAGES + text_npg; i < MEM_INVALID_PAGES + text_npg + data_bss_npg; i++) {
        pfn = getFreePhysicalFrame(&physicalFrames);
        if (pfn == -1) {
            TracePrintf(ERR, "No free physical frame\n");
            free(argbuf);
            close(fd);
            return (-1);
        }

        setPTE(&ptr0[i], getFreePhysicalFrame(&physicalFrames), 1, PROT_READ | PROT_WRITE, PROT_READ | PROT_WRITE);
    }

    /* And finally the user stack pages */
    // >>>> For stack_npg number of PTEs in the Region 0 page table
    // >>>> corresponding to the user stack (the last page of the
    // >>>> user stack *ends* at virtual address USER_STACK_LIMIT),
    // >>>> initialize each PTE:
    // >>>>     valid = 1
    // >>>>     kprot = PROT_READ | PROT_WRITE
    // >>>>     uprot = PROT_READ | PROT_WRITE
    // >>>>     pfn   = a new page of physical memory

    TracePrintf(TRC, "LoadProgram: set user stack pages %d ~ %d\n", (USER_STACK_LIMIT >> PAGESHIFT) - stack_npg, USER_STACK_LIMIT >> PAGESHIFT);
    for (i = (USER_STACK_LIMIT >> PAGESHIFT) - stack_npg; i < (USER_STACK_LIMIT >> PAGESHIFT); i++) {
        pfn = getFreePhysicalFrame(&physicalFrames);
        if (pfn == -1) {
            TracePrintf(ERR, "No free physical frame\n");
            free(argbuf);
            close(fd);
            return (-1);
        }

        setPTE(&ptr0[i], pfn, 1, PROT_READ | PROT_WRITE, PROT_READ | PROT_WRITE);
    }

    /*
     *  All pages for the new address space are now in place.  Flush
     *  the TLB to get rid of all the old PTEs from this process, so
     *  we'll be able to do the read() into the new pages below.
     */

    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);


    TracePrintf(TRC, "LoadProgram: flush TLB\n");
    /*
     *  Read the text and data from the file into memory.
     */
    if (read(fd, (void *)MEM_INVALID_SIZE, li.text_size+li.data_size)
	    != li.text_size+li.data_size) {
        
        TracePrintf(ERR, "LoadProgram: couldn't read for '%s'\n", name);
        free(argbuf);
        close(fd);
        // >>>> Since we are returning -2 here, this should mean to
        // >>>> the rest of the kernel that the current process should
        // >>>> be terminated with an exit status of ERROR reported
        // >>>> to its parent process.
        return (-2);
    }

    close(fd);			/* we've read it all now */

    /*
     *  Now set the page table entries for the program text to be readable
     *  and executable, but not writable.
     */
    // >>>> For text_npg number of PTEs corresponding to the user text
    // >>>> pages, set each PTE's kprot to PROT_READ | PROT_EXEC.

    TracePrintf(TRC, "LoadProgram: set program text pages %d ~ %d\n", MEM_INVALID_PAGES, MEM_INVALID_PAGES + text_npg);
    for (i = MEM_INVALID_PAGES; i < MEM_INVALID_PAGES + text_npg; i++) {
        ptr0[i].kprot = PROT_READ | PROT_EXEC;
    }

    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

    /*
     *  Zero out the bss
     */
    memset((void *)(MEM_INVALID_SIZE + li.text_size + li.data_size),
	    '\0', li.bss_size);

    /*
     *  Set the entry point in the ExceptionInfo.
     */
    // >>>> Initialize pc for the current process to (void *)li.entry

    info->pc = (void *)li.entry;

    /*
     *  Now, finally, build the argument list on the new stack.
     */

    *cpp++ = (char *)argcount;		/* the first value at cpp is argc */
    cp2 = argbuf;
    for (i = 0; i < argcount; i++) {      /* copy each argument and set argv */
        TracePrintf(LOG, "%d\n", i);
        *cpp++ = cp;
        strcpy(cp, cp2);
        cp += strlen(cp) + 1;
        cp2 += strlen(cp2) + 1;
    }
    

    free(argbuf);
    *cpp++ = NULL;	/* the last argv is a NULL pointer */
    *cpp++ = NULL;	/* a NULL pointer for an empty envp */
    *cpp++ = 0;		/* and terminate the auxiliary vector */

    /*
     *  Initialize all regs[] registers for the current process to 0,
     *  initialize the PSR for the current process also to 0.  This
     *  value for the PSR will make the process run in user mode,
     *  since this PSR value of 0 does not have the PSR_MODE bit set.
     */
    // >>>> Initialize regs[0] through regs[NUM_REGS-1] for the
    // >>>> current process to 0.
    // >>>> Initialize psr for the current process to 0.

    for (i = 0; i < NUM_REGS; i++) info->regs[i] = 0;

    info->psr = 0;

    TracePrintf(LOG, "LoadProgram Success\n");

    return (0);
}