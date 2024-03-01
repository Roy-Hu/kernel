#include "global.h"

int totalPhysicalFrameNum;

TrapHandlerPtr interruptVectorTable[TRAP_VECTOR_SIZE];

PhysicalFrame physicalFrames;

PTE *ptr0, ptr1[PAGE_TABLE_LEN];

int vmEnable;

void *kernelBreak;