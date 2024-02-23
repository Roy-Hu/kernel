#include <comp421/hardware.h>
#include <comp421/yalnix.h>

//  3.1 Yalnix Kernel Calls
int Fork(void);

int Exec(char *filename, char **argvec);

void Exit(int status);

int Wait(int *status_ptr);

int GetPid(void);

int Brk(void *addr);

int Delay (int clock_ticks);

int TtyRead(int tty_id, void *buf, int len);

int TtyWrite(int tty_id, void *buf, int len);