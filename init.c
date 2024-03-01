#include <comp421/hardware.h>
#include <comp421/yalnix.h>

#include "global.h"

int main(int argc, char *argv[]) {
    TracePrintf(LOG, "Run Init Process\n");
    GetPid();
	while(1) Pause();
}