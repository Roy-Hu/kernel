#include <comp421/hardware.h>
#include <stdbool.h>

#include "trap.h"

typedef struct pcb {
} PCB;

typedef struct physicalPage {
    int freePageNum;
    bool *isFree;
} PhysicalPage;

void *myMalloc(size_t size);

void initInterruptVectorTable(TrapHandlerPtr *InterruptVectorTable);

void initFreePhysicalPage(PhysicalPage *PhysicalPages, unsigned int pmem_size, void *orig_brk);