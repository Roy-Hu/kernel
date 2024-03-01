#include <comp421/hardware.h>
#include "global.h"

int main(int argc, char *argv[]) {
    TracePrintf(LOG, "Run Idle Process\n");
	while(1) Pause();
}