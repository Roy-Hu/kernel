#include <comp421/hardware.h>

//  3.2 Interrupt,Exception,and Trap Handling

typedef void (*TrapHandlerPtr)(ExceptionInfo *);

void TrapKernelHandler(ExceptionInfo *);

void TrapClockHandler(ExceptionInfo *);

void TrapIllegalHandler(ExceptionInfo *);

void TrapMemoryHandler(ExceptionInfo *);

void TrapMathHandler(ExceptionInfo *);

void TrapTtyReceiveHandler(ExceptionInfo *);

void TrapTtyTransmitHandler(ExceptionInfo *);
