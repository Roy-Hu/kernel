#ifndef CALL_H_
#define CALL_H_

#include <comp421/hardware.h>

int MyFork(void);

void MyExit(int status);

int MyExec(char *filename, char **argvec, ExceptionInfo *info);

int MyWait(int *status_ptr);

int MyGetPid(void);

int MyBrk(void *addr);

int MyDelay (int clock_ticks);

int MyTtyRead(int tty_id, void *buf, int len);

int MyTtyWrite(int tty_id, void *buf, int len);

#endif
