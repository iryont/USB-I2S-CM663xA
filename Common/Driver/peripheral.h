#ifndef _PERIPHERAL_H_
#define _PERIPHERAL_H_

#include "common.h"

//-----------------------------------------------------------------------------
// IO Confiuration
//-----------------------------------------------------------------------------
#define _SPI_
#define _I2C_MASTER_
#define _HDA_LINK_

//-----------------------------------------------------------------------------
// Public Variable
//-----------------------------------------------------------------------------
// INT1 sources
extern volatile BOOL g_I2cSlaveRequest;
extern volatile BOOL g_HdaRequest;
extern volatile BOOL g_SpdifInRequest;
extern volatile BOOL g_GpiRequest;
extern volatile BOOL g_GpioRequest;

//-----------------------------------------------------------------------------
// Public Function
//-----------------------------------------------------------------------------
extern BYTE FlashRead(WORD addr);

#ifdef _HDA_LINK_
extern void SendHdaCommand(BYTE node, WORD verb, WORD payload);
#endif

#ifdef _SPI_
extern void SpiWriteByte(BYTE chip_select, BYTE map, BYTE value);
extern BYTE SpiReadByte(BYTE chip_select, BYTE map);
#endif

#ifdef _I2C_MASTER_
#define I2C_FAST_MODE bmBIT7
#define I2C_CLOCK_SYNC bmBIT6

// The maximum of len is 8.
// If the bit7 of paramter len is 1, I2C will run in 400k fast mode, else I2C runs in 100k standard mode.
// If the bit6 of paramter len is 1, I2C will turn on clock sync. mode.
extern BOOL I2cMasterWrite(BYTE addr, BYTE map, BYTE* buf, BYTE len);
extern BOOL I2cMasterRead(BYTE addr, BYTE map, BYTE* buf, BYTE len);
#endif

#endif

