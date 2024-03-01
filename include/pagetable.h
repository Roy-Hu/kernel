#include <comp421/hardware.h>

void freePhysicalFrame(int pfn);

int getFreePhysicalFrame();

void setPTE(struct pte *entry, int pfn, int valid, int uprot, int kprot);