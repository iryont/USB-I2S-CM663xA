#ifndef _UART_H_
#define _UART_H_

#include "common.h"

//-----------------------------------------------------------------------------
// Public Function
//-----------------------------------------------------------------------------
extern void UartInit();

extern BOOL TxQueueIsFull();
extern BOOL RxQueueIsEmpty();

extern BYTE GetFromRxQueue();
extern void PutIntoTxQueue(BYTE value);

#endif

