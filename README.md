# COMP 521 Lab2  The Yalnix Operating System Kernel
## TODO:
- Maybe need to fix the bugs in wait to passed that test?
- Review on context swictching functions
- Review on error handling
- Clean the style
- More to be added...
## Done:
- <del>Read this document carefully. Try to understand the details described in it, and perhaps read this
document again. Ask a TA or the instructor if you don’t understand parts of this document. Although
there are a lot of pages here to read, this document is intended to be very complete and to guide you
through the project.<del>
- <del>Carefully read through the provided header file hardware.h. This file is located in the directory
/clear/courses/comp421/pub/include/comp421. It defines many things you will need
and many things you will find useful in this project. Do not copy this file into your own directory.<del>
- <del>Think through and sketch out some high-level pseudo-code for at least some of the kernel calls,
interrupts, and exceptions. Then decide on the things you need to put in what data structures (notably
in the Process Control Block, or PCB) to make it all work. Iterate until the pseudo-code and the main
prototype data structures mesh well together. Do not worry whether you have everything you will
eventually need defined in your PCB structure; you can always add fields to the PCB later (although
you must be careful that you recompile everything that uses the PCB definition if you add or change
fields in the PCB structure definition).<del>
- <del>In designing your kernel data structures, think carefully about the difference in the C programming
language between a data structure created as an “automatic” variable, one created as a “static” or
external variable, and one whose storage was allocated by malloc. In particular, think about where
the memory (the storage) for each of these types of variables is located and when (if ever) that memory
will be freed. Be careful in which way you create each of your kernel’s data structures.<del>
- <del>Read about the tracing feature using TracePrintf, described in Section 8.2, that you can use in
this project. As you work on the project, particularly during debugging, you may find this feature
very useful; with it, you can, for example, on the command line dynamically turn up or down (or on
or off) different types of debugging tracing of your kernel or Yalnix user programs (or the RCS 421
hardware).<del>
- <del>Take a look at the template LoadProgram function in the file load. template. You need not
understand all the details, but make sure you understand the comments that are preceded by “>>>>”
markers.<del>
- <del>Do not copy any of the provided header files (hardware.h and yalnix.h) into your own directory.<del>
- <del>Write an initial version of your KernelStart function to bootstrap the kernel. You need to initialize
the interrupt vector table here and point the REG_VECTOR_BASE register at it. Note, however,
do not start out with an empty procedure for each of your interrupt, exception, or trap handlers, as
doing so can make your subsequent debugging sometimes very confusing. Instead, it is highly recommended
that you write a separate handler procedure for each defined entry in the interrupt vector
table, with at least a TracePrintf call and a Halt in it; you can then replace that Halt with
the real handler code in that procedure once you have written it. In your KernelStart function,
you also need to build the initial page tables and enable virtual memory by writing a 1 into the
REG_VM_ENABLE register. The support code that makes the RCS 421 simulation work does a lot of
error checking at this point, so if you get through this far with no errors, you are probably doing OK
(although we can’t check for all possible errors at this point).<del>
- <del>Write an idle program (a single infinite loop that just executes a Pause instruction). Try running
your kernel with just an idle process to see if that much works. The idle process should have Yalnix
process ID 0.<del>
- <del>Write a simple init program. The init process should have Yalnix process ID 1. The simplest init
program would just loop forever, but this form of the init program would generally only be useful for
testing; normally (for example, as suggested in the following steps), init should do something “real,”
including calling some Yalnix kernel calls, creating other processes, etc. Make sure you can get your
Makefile to compile this as a Yalnix user program, since you will need this skill in order to write
other test programs later. Modify your KernelStart to start this init program (or one passed on
the Linux shell command line) in addition to the idle program.<del>
- <del>Implement the GetPid kernel call and call it from the init process. At this point your kernel call
interface would seem to be working correctly.<del>
- **I will work on those functions, if I have finished, I will continue working on the rest. If you would like to, you can work on the rest also. But I think you have done a lot, great work!**:
  - <del> Implement the Delay kernel call and call it from the init process. Make sure your idle process then
  runs for several clock ticks, until the delay period expires. This will be the first proof that blocking
  of processes works and context switching work. <del>
  - <del> Try Delay several times in the same program, to make sure your init and idle processes can correctly
  context switch back and forth several times. At this point you will have basically achieved the
  recommended checkpoint described in Section 5. <del>
  - <del>Implement SetKernelBrk to allow your kernel to allocate substantial chunks of memory. It is
  likely that you haven’t needed it up to this point, but you may have (in this case implement it earlier).
  - <del>Implement the Brk kernel call and call it from the init process. At this point you have a substantial
  part of the memory management code working. <del>
- <del>Implement theFork kernel call. If you get this to work you are almost done with the memory system. <del>
- <del>Implement the Exec kernel call. You have already done something similar by initially loading init. <del>
- <del>Write another small programthat does not domuch. Call Fork and Exec from your init process, to
get this third program running as a child of init. Watch for context switches.<del>
- <del>Implement and test the Exit and Wait kernel calls.<del>
- <del>Implement the kernel calls related toterminal I/Ohandling. These should be easy at this point, if you
pay attention to the process address space into which your input needs to go.<del>

