#include <comp421/hardware.h>

#include "trap.h"

typedef struct pcb {
} PCB;

typedef struct physicalPage {
    int freePageNum;
    int *isFree;
} PhysicalPage;

typedef struct pageTable {
    struct pte region0[PAGE_TABLE_LEN], region1[PAGE_TABLE_LEN];
} PageTable;

void *myMalloc(size_t size);

void initInterruptVectorTable(TrapHandlerPtr *InterruptVectorTable);

void initFreePhysicalPage(PhysicalPage *PhysicalPages, unsigned int pmem_size, void *orig_brk);

void initPageTable(PageTable *pageTable, void *orig_brk);