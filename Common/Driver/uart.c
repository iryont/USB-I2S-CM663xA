#include "uart.h"

//-----------------------------------------------------------------------------
// Data Type Definition
//-----------------------------------------------------------------------------
#define TX_BUF_SIZE 4
#define TX_BUF_MASK 3	// BUF_SIZE-1

#define RX_BUF_SIZE 4
#define RX_BUF_MASK 3	// BUF_SIZE-1

typedef struct
{
	BYTE head;
	BYTE tail;
	BYTE buf[TX_BUF_SIZE];
} UART_TX_QUEUE;

typedef struct
{
	BYTE head;
	BYTE tail;
	BYTE buf[RX_BUF_SIZE];
} UART_RX_QUEUE;

//-----------------------------------------------------------------------------
// Static Variables
//-----------------------------------------------------------------------------
static UART_TX_QUEUE idata s_UartTxQueue;
static UART_RX_QUEUE idata s_UartRxQueue;

static volatile BOOL s_UartTxIdle = TRUE;

//-----------------------------------------------------------------------------
// Code
//-----------------------------------------------------------------------------
static void ISR_UART() interrupt 4
{
	if(TI)
	{
		if(s_UartTxQueue.head != s_UartTxQueue.tail)
		{
			SBUF = s_UartTxQueue.buf[s_UartTxQueue.tail];
			s_UartTxQueue.tail = (s_UartTxQueue.tail + 1) & TX_BUF_MASK;
			s_UartTxIdle = FALSE;
		}
		else
		{
			s_UartTxIdle = TRUE;
		}

		TI = 0;
	}

	if(RI)
	{
		s_UartRxQueue.buf[s_UartRxQueue.head] = SBUF;
		s_UartRxQueue.head ++;
		s_UartRxQueue.head &= RX_BUF_MASK;

		RI = 0;
	}
}

void UartInit()
{
	s_UartTxQueue.head = s_UartTxQueue.tail = 0;
	s_UartRxQueue.head = s_UartRxQueue.tail = 0;	
}

BOOL TxQueueIsFull()
{
	return (s_UartTxQueue.head == ((s_UartTxQueue.tail-1) & TX_BUF_MASK));
}

BOOL RxQueueIsEmpty()
{
	return (s_UartRxQueue.head == s_UartRxQueue.tail);
}

/*BOOL TxQueueIsEmpty()
{
	return (s_UartTxQueue.head == s_UartTxQueue.tail);
}*/

BYTE GetFromRxQueue()
{
	g_TempByte1 = s_UartRxQueue.buf[s_UartRxQueue.tail];
	s_UartRxQueue.tail = (s_UartRxQueue.tail + 1) & RX_BUF_MASK;

	return g_TempByte1;
}

/*BYTE GetFromTxQueue()
{
	g_TempByte1 = s_UartTxQueue.buf[s_UartTxQueue.tail];
	s_UartTxQueue.tail = (s_UartTxQueue.tail + 1) & TX_BUF_MASK;

	return g_TempByte1;
}*/

/*void PutIntoRxQueue(BYTE value)
{
	s_UartRxQueue.buf[s_UartRxQueue.head] = value;
	s_UartRxQueue.head ++;
	s_UartRxQueue.head &= RX_BUF_MASK;
}*/

void PutIntoTxQueue(BYTE value)
{
	s_UartTxQueue.buf[s_UartTxQueue.head] = value;
	s_UartTxQueue.head ++;
	s_UartTxQueue.head &= TX_BUF_MASK;

	if(s_UartTxIdle)
	{
		TI = 1;
	}
}

