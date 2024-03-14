#include <stdio.h>
#include <stdlib.h>
#include <comp421/yalnix.h>
#include <comp421/hardware.h>

int
main(int argc, char **argv)
{
    fprintf(stderr, "Starting delay...\n");
    Delay(5);
    fprintf(stderr, "Delay finished!\n");

    Exit(0);
}
