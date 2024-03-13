#ifndef TRAP_H_
#define TRAP_H_

#include <comp421/hardware.h>

typedef void (*TrapHandlerPtr)(ExceptionInfo *);

extern TrapHandlerPtr interruptVectorTable[TRAP_VECTOR_SIZE];

//  3.2 Interrupt,Exception,and Trap Handling
void TrapKernelHandler(ExceptionInfo *);

void TrapClockHandler(ExceptionInfo *);

void TrapIllegalHandler(ExceptionInfo *);

void TrapMemoryHandler(ExceptionInfo *);

void TrapMathHandler(ExceptionInfo *);

void TrapTtyReceiveHandler(ExceptionInfo *);

void TrapTtyTransmitHandler(ExceptionInfo *);

#endif