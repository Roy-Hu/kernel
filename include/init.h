#include <comp421/hardware.h>

#include "kernel.h"
#include "trap.h"

void initInterruptVectorTable(TrapHandlerPtr *);

void *initFreePhysicalFrame(PhysicalFrame *, int, void *);

void initPageTable(PageTable *, PageTable *, void *);
