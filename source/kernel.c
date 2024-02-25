#include "kernel.h"

void freePhysicalFrame(PhysicalFrame *physicalFrame, int pfn) {
    physicalFrame->isFree[pfn] = 1;
    physicalFrame->freePFN++;
}

int getFreePhysicalFrame(PhysicalFrame *physicalFrame) {
    int pfn;
    for (pfn = 0; pfn < physicalFrame->totalPFN; pfn++) {
        if (physicalFrame->isFree[pfn] == 1) {
            physicalFrame->isFree[pfn] = 0;
            physicalFrame->freePFN--;
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