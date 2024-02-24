#include <comp421/hardware.h>

#include "kernel.h"
#include "trap.h"

void initInterruptVectorTable(TrapHandlerPtr *InterruptVectorTable);

void *initFreePhysicalFrame(PhysicalFrame *PhysicalFrames, int totalPhysicalFrameNum, void *orig_brk);

void *initPageTable(PageTable *region0, PageTable *region1, PhysicalFrame *PhysicalFrames, void *orig_brk);
