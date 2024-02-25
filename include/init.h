#include <comp421/hardware.h>

#include "kernel.h"
#include "trap.h"

void initInterruptVectorTable(TrapHandlerPtr *);

void *initFreePhysicalFrame(PhysicalFrame *, int, void *);

PTE *initPageTable(PTE *, void *);
