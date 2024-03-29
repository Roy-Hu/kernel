#ifndef PCB_H_
#define PCB_H_

#include <comp421/hardware.h>
#include "pagetable.h"

typedef enum {
  WAITING,
  WAITCHILD,
  RUNNING,
  READY,
  TERMINATED
}STATE;

#define IDLE_PID 0
#define INIT_PID 1

typedef struct exitChildStatus {
    int pid;
    int status;
    struct exitChildStatus *next;
} exitChildStatus;

typedef struct pcb {
    int pid;

    int brk;

    struct pcb *parent;
    
    struct pcb *child;

    struct pcb *sibling;

    int childNum;

    exitChildStatus *exitChild;

    int stackTop;

    STATE state;
    
    int readyTime;

    PTE *ptr0;
    
    SavedContext* ctx;

    struct pcb *next;
} PCB;


extern PCB *readyPCBTail, *waitingPCBTail;

extern PCB *read_queue[NUM_TERMINALS];

extern PCB *write_queue[NUM_TERMINALS];

extern PCB *runningPCB, *waitingPCB, *readyPCB, *idlePCB, *initPCB;

PCB *createPCB(int pid);

void addSibling(PCB *parent, PCB *sibling);

void removeSibling(PCB *parent, PCB *child);

void terminateProcess(PCB *pcb, int status);

void add_to_read_queue(PCB *proc, int term_id);

void add_to_write_queue(PCB *proc, int term_id);

PCB *pop_writing_queue(PCB *p);

PCB *pop_read_queue(PCB *p);

void pushExitStatus(PCB *pcb, int pid, int status);

exitChildStatus *popExitStatus(PCB *pcb);

void freeExitStatus(PCB *pcb);

void pushPCB(PCB *pcb);

PCB *popPCB(STATE state);

SavedContext *test_init(SavedContext *ctxp, void *p1, void *p2);

SavedContext *switch_func(SavedContext *ctxp, void *p1, void *p2);

SavedContext *switch_clock_trap(SavedContext *ctxp, void *p1, void *p2);

SavedContext *switch_fork(SavedContext *ctxp, void *p1, void *p2);

#endif