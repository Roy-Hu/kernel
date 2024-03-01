#include "global.h"

int totalPhysicalFrameNum;

TrapHandlerPtr interruptVectorTable[TRAP_VECTOR_SIZE];

PhysicalFrame physicalFrames;

PTE *ptr0, ptr1[PAGE_TABLE_LEN];

int vmEnable = 0;

void *kernelBreak;

PCB *currentPCB, *idlePCB;

// 0 for idle, 1 for init
int processId = 2;