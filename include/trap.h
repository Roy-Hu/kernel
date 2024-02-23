#include <comp421/hardware.h>

//  3.2 Interrupt,Exception,and Trap Handling

typedef void (*TrapHandlerPtr)(ExceptionInfo *);

void TrapKernelHandler(ExceptionInfo *info);

void TrapClockHandler(ExceptionInfo *info);

void TrapIllegalHandler(ExceptionInfo *info);

void TrapMemoryHandler(ExceptionInfo *info);

void TrapMathHandler(ExceptionInfo *info);

void TrapTtyReceiveHandler(ExceptionInfo *info);

void TrapTtyTransmitHandler(ExceptionInfo *info);
