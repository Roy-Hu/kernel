#include <comp421/hardware.h>

#define ERR 0
#define WAR 1
#define LOG 2
#define INF 3
#define TRC 4

typedef struct pcb {
} PCB;

typedef struct pte PageTable;

typedef struct physicalFrame {
    int freePageNum;
    int *isFree;
} PhysicalFrame;