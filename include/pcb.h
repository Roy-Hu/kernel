#ifndef PCB_H_
#define PCB_H_

#include <comp421/hardware.h>
#include "pagetable.h"

typedef enum {
  WAITING,
  RUNNING,
  READY,
  TERMINATED
}STATE;

#define IDLE_PID 0
#define INIT_PID 1

typedef struct pcb {
    int pid;

    // physical address of pt0
    int pt0addr;

    // pt0 >> PAGESHIFT
    int pt0pfn;
    
    int brk;

    struct pcb *parent;

    int stackTop;

    STATE state;
    
    int readyTime;

    // ptr0
    PTE *ptr0;
    
    SavedContext* ctx;

    struct pcb *next;
} PCB;


extern PCB *readyPCBTail, *waitingPCBTail;

extern PCB *runningPCB, *waitingPCB, *readyPCB, *idlePCB, *initPCB;

PCB *createPCB(int pid);

void pushPCB(PCB *pcb);

PCB *popPCB(STATE state);

SavedContext *test_init(SavedContext *ctxp, void *p1, void *p2);

SavedContext *switch_func(SavedContext *ctxp, void *p1, void *p2);

SavedContext *switch_clock_trap(SavedContext *ctxp, void *p1, void *p2);

SavedContext *switch_fork(SavedContext *ctxp, void *p1, void *p2);

#endif