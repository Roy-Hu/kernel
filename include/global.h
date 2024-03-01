#include <comp421/hardware.h>

#define ERR 0
#define WAR 1
#define LOG 2
#define INF 3
#define TRC 4

typedef void (*TrapHandlerPtr)(ExceptionInfo *);

typedef struct pcb {
    int pid;
    SavedContext ctx;

    struct pcb *next;
} PCB;

typedef struct pte PTE;

typedef struct physicalFrame {
    int totalPFN;
    int freePFN;
    int *isFree;
} PhysicalFrame;


extern int totalPhysicalFrameNum;

extern TrapHandlerPtr interruptVectorTable[TRAP_VECTOR_SIZE];

extern PhysicalFrame physicalFrames;

extern PTE *ptr0, ptr1[PAGE_TABLE_LEN];

extern int vmEnable;

extern void *kernelBreak;

extern PCB *currentPCB, *idlePCB;

extern int processId;