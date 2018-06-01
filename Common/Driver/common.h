#ifndef _COMMON_H_
#define _COMMON_H_

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define TRUE  1
#define FALSE 0

#define NULL 0

#define bmBIT0 0x01
#define bmBIT1 0x02
#define bmBIT2 0x04
#define bmBIT3 0x08
#define bmBIT4 0x10
#define bmBIT5 0x20
#define bmBIT6 0x40
#define bmBIT7 0x80

#define MSB(word) (BYTE)(((WORD)(word) >> 8) & 0x00ff)
#define LSB(word) (BYTE)((WORD)(word) & 0x00ff)
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

//-----------------------------------------------------------------------------
// Type Definition
//-----------------------------------------------------------------------------
typedef unsigned char BYTE;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef bit BOOL;

typedef union
{
   DWORD _long[4];
   BYTE _byte[16];
} BUFFER_16BYTES;

//-----------------------------------------------------------------------------
// Public Variable
//-----------------------------------------------------------------------------
extern BYTE g_Index, g_TempByte1, g_TempByte2, g_TempByte3;
extern WORD g_TempWord1, g_TempWord2;
extern void* g_Pointer;

//-----------------------------------------------------------------------------
// Public Function
//-----------------------------------------------------------------------------
extern void PERI_WriteByte(WORD addr, BYTE value);
extern void PERI_WriteWord(WORD addr, WORD value);
extern BYTE PERI_ReadByte(WORD addr);
extern WORD PERI_ReadWord(WORD addr);

//-----------------------------------------------------------------------------
// Common Head File
//-----------------------------------------------------------------------------
#include "cm66xx.h"

#endif

