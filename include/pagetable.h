#include <comp421/hardware.h>

#define INIT_PT0_PFN (2 * PAGE_TABLE_LEN - 1)
#define PT0_VPN (PAGE_TABLE_LEN - 1)

typedef struct pte PTE;

extern PTE *ptr0, ptr1[PAGE_TABLE_LEN];

extern void *kernelBreak;

typedef struct physicalFrame {
    int totalPFN;
    int freePFN;
    int *isFree;
} PhysicalFrame;

extern int totalPhysicalFrameNum;

extern PhysicalFrame physicalFrames;

void freePhysicalFrame(int pfn);

int getFreePhysicalFrame();

void setPTE(struct pte *entry, int pfn, int valid, int uprot, int kprot);