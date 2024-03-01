#include "pagetable.h"
#include "global.h"

void freePhysicalFrame(int pfn) {
    physicalFrames.isFree[pfn] = 1;
    physicalFrames.freePFN++;
}

int getFreePhysicalFrame() {
    int pfn;
    for (pfn = 0; pfn < physicalFrames.totalPFN; pfn++) {
        if (physicalFrames.isFree[pfn] == 1) {
            physicalFrames.isFree[pfn] = 0;
            physicalFrames.freePFN--;
            return pfn;
        }
    }
    
    return -1;
}

void setPTE(struct pte *entry, int pfn, int valid, int uprot, int kprot) {
    entry->pfn = pfn;
    entry->valid = valid;
    entry->uprot = uprot;
    entry->kprot = kprot;
}