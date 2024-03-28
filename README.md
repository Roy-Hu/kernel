# COMP 521 Lab2  The Yalnix Operating System Kernel

### Introduction

Yalnix is an operating system kernel running on a simulated computer system hardware RCS421. In this project, we have implemented several kernel calls and traps, memory management, context switch, bootstrapping, and loading user programs.

### Authors

* Chien-An Hu(ch155), Qiyuan Yang(qy28)

### Structures

### Source
#### init.c 
The `init.c` file contains essential functions required for bootstrapping the system and initializing the kernel. Key functionalities include:

- **Interrupt Vector Table Initialization:** Sets up the interrupt vector table to handle interrupts and exceptions efficiently.
- **Physical Frames Initialization:** Allocates and manages free physical frames for system processes and kernel components.
- **Page Tables Initialization:** Establishes the initial page tables required for virtual memory management.
  
#### pagetable.c 
The `pagetable.c` file includes functions critical for managing the page table, which is pivotal for memory allocation and management. Key features are:

- **Page Table Allocation:** Allocates memory for new page tables, facilitating the expansion of virtual memory as needed.
- **Frame Management:** Includes functions to allocate, free, and manage frames within the system, ensuring efficient memory usage.
- **PTE Setup:** Provides mechanisms to set up and manage page table entries, allowing for detailed control over memory access and permissions.

#### pcb.c
The `pcb.c` file includes functions for creating, managing, and terminating processes. It handles the intricacies of process state transitions, including ready, running, waiting, and terminated states. Additionally, it manages queues for ready and waiting processes, as well as read and write queues for terminal I/O operations.

- **Process Creation and Termination:** Create new PCBs for processes and handle their termination, including cleaning up resources and updating parent-child relationships.
- **Context Switching:** Context switching to transition between processes, supporting operations like fork, clock interrupts, and explicit switches.
- **Queue Management:** Manages multiple queues to organize processes based on their states (ready, waiting) and activities (reading from or writing to terminals).
- **Parent-Child Management:** Manage parent and child processes, handling exit statuses and ensuring proper state transitions.

#### trap.c
The `trap.c` file contains the handlers for different types of traps, including system calls, clock interrupts, illegal operations, memory access violations, mathematical errors, and terminal I/O operations.

- **TrapKernelHandler:** Handles system calls for process control (FORK, EXEC, EXIT, WAIT), memory management (BRK), process identification (GETPID), scheduling (DELAY), and terminal I/O (TTY_READ, TTY_WRITE).
- **TrapClockHandler:** Manages clock interrupts, updating the system time and moving processes between waiting and ready queues as appropriate.
- **TrapIllegalHandler:** Catches illegal instructions or operations performed by user programs, including illegal opcodes, operands, addressing modes, software traps, and privileged instructions.
- **TrapMemoryHandler:** Handles memory access violations and page faults, performs dynamic memory allocation, and protects against unauthorized access.
- **TrapMathHandler:** Addresses errors related to mathematical operations, such as integer divide by zero, overflow, underflow, and invalid floating-point operations.
- **TrapTtyReceiveHandler:** Manages terminal input operations, reading from terminal devices and handling input-related interrupts.
- **TrapTtyTransmitHandler:** Handles terminal output operations, managing the transmission of data to terminal devices and related interrupts.

#### terminal.c
The `terminal.c` file initialization global variable use by the terminal.

#### global.c
The `global.c` file contains global variable use by the kernel.

## TODO:
- Review on context swictching functions
- **Review on error handling**
- **Rewrite a readme to give an introduction of our structure**
- Clean the style
- More to be added...

