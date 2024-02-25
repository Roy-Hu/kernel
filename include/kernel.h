#include <comp421/hardware.h>

#define ERR 0
#define WAR 1
#define LOG 2
#define INF 3
#define TRC 4

typedef struct pcb {
    int pid;
} PCB;

typedef struct pte PTE;

typedef struct physicalFrame {
    int totalPFN;
    int freePFN;
    int *isFree;
} PhysicalFrame;

void freePhysicalFrame(PhysicalFrame *physicalFrame, int pfn);

int getFreePhysicalFrame(PhysicalFrame *physicalFrame);

void setPTE(struct pte *entry, int pfn, int valid, int uprot, int kprot);