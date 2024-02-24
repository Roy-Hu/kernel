#include <comp421/hardware.h>

#include "trap.h"

#define LOG 0
#define ERR 1
#define WAR 2
#define INF 3
#define TRC 4

typedef struct pcb {
} PCB;

typedef struct pte PageTable;
typedef struct physicalPage {
    int freePageNum;
    int *isFree;
} PhysicalPage;

void *myMalloc(size_t size);

void initInterruptVectorTable(TrapHandlerPtr *InterruptVectorTable);

void initFreePhysicalPage(PhysicalPage *PhysicalPages, unsigned int pmem_size, void *orig_brk);

void initPageTable(PageTable *region0, PageTable *region1, void *orig_brk);
