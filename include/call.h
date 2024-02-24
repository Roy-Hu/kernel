#include <comp421/hardware.h>
#include <comp421/yalnix.h>

//  3.1 Yalnix Kernel Calls
int Fork(void);

int Exec(char *, char **);

void Exit(int);

int Wait(int *);

int GetPid(void);

int Brk(void *);

int Delay (int);

int TtyRead(int, void *, int);

int TtyWrite(int, void *, int);