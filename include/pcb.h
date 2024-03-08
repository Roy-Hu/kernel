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
    
    int pt0addr;

    int pt0pfn;
    
    int brk;

    int stackTop;

    STATE state;
    
    int readyTime;

    PTE *ptr0;
    
    SavedContext ctx;

    struct pcb *next;
} PCB;


extern PCB *readyPCBTail, *waitingPCBTail;

extern PCB *runningPCB, *waitingPCB, *readyPCB, *idlePCB, *initPCB;

PCB *createPCB(int pid);

void pushPCB(PCB *pcb);

PCB *popPCB(STATE state);

SavedContext *MySwitchFunc(SavedContext *ctxp, void *p1, void *p2);