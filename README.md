# COMP 521 Lab2  The Yalnix Operating System Kernel
## TODO:
### kernel start
 - <del>Initialize the interrupt vector table entries for each type of interrupt, exception, or trap, by making 
 them point to the correct handler functions in your kernel.<del>
 - <del>Initialize the REG_VECTOR_BASEprivileged machine register to point to your interrupt vector table.<del>
 - <del>Build a structure to keep track of what page frames in physical memory are free. For this purpose,
 you might be able to use a linked list of physical frames, implemented in the frames themselves. Or
 you can have a separate structure, which is probably easier, though slightly less efficient. This list of
 free page frames should be based on the pmem_size argument passed to your KernelStart,but
 be careful not include any memory that is already in use by your kernel.<del>
 - <del>Also, as your kernel later allocates and frees pages of physical memory, be very careful not to accidentally
 end up using the same page of physical memory twice for different uses at the same time; for
 example, be careful when you allocate a free page of physical memory, to actually remove that page
 from your list of free physical pages, and when you free a page of physical memory, to actually add
 that page back correctly (and only once) to your list of free physical pages. If you do end up using
 the same page of physical memory twice for different uses at the same time, the results can be very
 strange and difficult to debug (think about why this is so).
 - <del>Build the initial page tables for Region 0 and Region 1, and initialize the registers REG_PTR0 and
 REG_PTR1to define these initial page tables.<del>
 - <del>Enable virtual memory.<del>
 - <del>Create an “idle” process to be run by your kernel when there are no other runnable (ready) processes
 in the system. The idle process should be a loop that executes the Pause machine instruction on each
 loop iteration. The idle process can be loaded from a file using LoadProgram, in the same way as
 a normal user process. Or, if you think about it, it is possible to have the code for the idle process
 “built into” the rest of the code for your kernel, for example by placing the idle process’s code in a
 separate C procedure, so that the machine language code for the idle process then automatically gets
 loaded into memory along with the rest of your kernel; you wouldn’t actually call this procedure, but
 rather you can initialize the pc value for the idle process to the address of this code for idle.<del>
 - <del>Create the first “regular” process, known as the init process, and load the initial program into it. In
 this step, guide yourself by the file load.template that we provide, which shows a skeleton of
 the procedure necessary to load an executable program from a Linux file into memory as a Yalnix
 process. This initial process will serve a role similar to that of the “init” process in Unix/Linux as
 the parent (or grandparent, etc.) of all processes running on the system. However, in Yalnix, unlike
 in Unix and Linux, when a process’s parent exits, that process is not “inherited by” the init process.
 Rather, as noted in Section 3.1, that process continues to run but simply has no parent process. To
 run your initial program you should put the file name of the init program on your shell command line
 when you run your kernel. This program name will then be passed to your KernelStartas one of
 the cmd_argsstrings.<del>
 - <del>Return from your KernelStart routine. The machine will begin running the program defined by
 the current page tables and by the values returned in the ExceptionInfo structure (values which
 you have presumably modified to point to the program counter and stack pointer, etc., of the initial
 process)<del>
## Issue:
