#include "pagetable.h"
#include "global.h"

int totalPhysicalFrameNum;

PhysicalFrame physicalFrames;

PTE *ptr0, ptr1[PAGE_TABLE_LEN];

void *kernelBreak;

void freePhysicalFrame(int pfn) {
    if (physicalFrames.isFree[pfn] == 0) physicalFrames.freePFN++;
    physicalFrames.isFree[pfn] = 1;
}

int getFreePhysicalFrame() {
    int pfn;
    for (pfn = 0; pfn < physicalFrames.totalPFN; pfn++) {
        if (physicalFrames.isFree[pfn] == 1) {
            physicalFrames.isFree[pfn] = 0;
            physicalFrames.freePFN--;

            // TracePrintf(TRC, "Get free physical frame %d\n", pfn);
            return pfn;
        }
    }
    
    TracePrintf(ERR, "No free physical frame\n");

    return -1;
}

void setPTE(PTE *entry, int pfn, int valid, int uprot, int kprot) {
    entry->pfn = pfn;
    entry->valid = valid;
    entry->uprot = uprot;
    entry->kprot = kprot;
}