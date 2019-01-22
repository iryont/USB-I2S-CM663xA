#define ALLOCATE_EXTERN

#include "peripheral.h"

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
// INT1 sources
volatile BOOL g_I2cMasterRequest = FALSE;
volatile BOOL g_I2cSlaveRequest = FALSE;
volatile BOOL g_GpioRequest = FALSE;
volatile BOOL g_HdaRequest = FALSE;
volatile BOOL g_UsbSuspendRequest = FALSE;
volatile BOOL g_UsbResumeRequest = FALSE;
volatile BOOL g_UsbResetRequest = FALSE;
volatile BOOL g_SpdifInRequest = FALSE;
volatile BOOL g_GpiRequest = FALSE;
volatile BOOL g_SelfPowerRequest = FALSE;
volatile BOOL g_BulkOutRequest = FALSE;
volatile BOOL g_Interrupt4InComplete = FALSE;
volatile BOOL g_Interrupt15InComplete = FALSE;

BYTE g_Index, g_TempByte1, g_TempByte2;
WORD g_TempWord1, g_TempWord2;
void* g_Pointer;

//-----------------------------------------------------------------------------
// Code
//-----------------------------------------------------------------------------
static void ISR_INT1() interrupt 2
{
	BYTE status = INTERRUPT_STATUS_L;

	if(status & bmBIT0)
	{
		g_I2cMasterRequest = TRUE;
		INTERRUPT_CLEAR_L = bmBIT0;
	}

	if(status & bmBIT1)
	{
		g_I2cSlaveRequest = TRUE;
		INTERRUPT_CLEAR_L = bmBIT1;
	}

	if(status & bmBIT2)
	{
		g_GpioRequest = TRUE;
		INTERRUPT_CLEAR_L = bmBIT2;
	}

	if(status & bmBIT3)
	{
		g_HdaRequest = TRUE;
		INTERRUPT_CLEAR_L = bmBIT3;
	}

	if(status & bmBIT4)
	{
		g_UsbSuspendRequest = TRUE;
		INTERRUPT_CLEAR_L = bmBIT4;
	}

	if(status & bmBIT5)
	{
		g_UsbResumeRequest = TRUE;
		INTERRUPT_CLEAR_L = bmBIT5;
	}

	if(status & bmBIT6)
	{
		g_SpdifInRequest = TRUE;
		INTERRUPT_CLEAR_L = bmBIT6;
	}

	if(status & bmBIT7)
	{
		g_UsbResetRequest = TRUE;
		INTERRUPT_CLEAR_L = bmBIT7;
	}
	
	status = INTERRUPT_STATUS_H;
	if(status & bmBIT0)
	{
		g_GpiRequest = TRUE;
		INTERRUPT_CLEAR_H = bmBIT0;
	}

	if(status & bmBIT1)
	{
		g_SelfPowerRequest = TRUE;
		INTERRUPT_CLEAR_H = bmBIT1;
	}

	if(status & bmBIT2)
	{
		g_BulkOutRequest = TRUE;
		INTERRUPT_CLEAR_H = bmBIT2;
	}

	if(status & bmBIT3)
	{
		g_Interrupt4InComplete = TRUE;
		INTERRUPT_CLEAR_H = bmBIT3;
	}

	if(status & bmBIT4)
	{
		g_Interrupt15InComplete = TRUE;
		INTERRUPT_CLEAR_H = bmBIT4;
	}
}

void PERI_WriteByte(WORD addr, BYTE value)
{
	PERI_DEST_ADDR_L = LSB(addr);
	PERI_DEST_ADDR_H = MSB(addr);

	PERI_DATA_W_L = value;

	PERI_HANDSHAKE &= ~bmBIT2;
	PERI_HANDSHAKE = 0x04;

	//while(PERI_HANDSHAKE&bmBIT0);	// Useless
}

void PERI_WriteWord(WORD addr, WORD value)
{
	PERI_DEST_ADDR_L = LSB(addr);
	PERI_DEST_ADDR_H = MSB(addr);

	PERI_DATA_W_L = LSB(value);
	PERI_DATA_W_H = MSB(value);

	PERI_HANDSHAKE &= ~bmBIT2;
	PERI_HANDSHAKE = 0x14;

	//while(PERI_HANDSHAKE&bmBIT0);	// Useless
}

BYTE PERI_ReadByte(WORD addr)
{
	PERI_DEST_ADDR_L = LSB(addr);
	PERI_DEST_ADDR_H = MSB(addr);

	PERI_HANDSHAKE &= ~bmBIT2;
	PERI_HANDSHAKE = 0x0C;

	//while(PERI_HANDSHAKE&bmBIT0);	// Useless

	return PERI_DATA_R_L;
}

WORD PERI_ReadWord(WORD addr)
{
	PERI_DEST_ADDR_L = LSB(addr);
	PERI_DEST_ADDR_H = MSB(addr);

	PERI_HANDSHAKE &= ~bmBIT2;
	PERI_HANDSHAKE = 0x1C;

	//while(PERI_HANDSHAKE&bmBIT0);	// Useless

	return ((PERI_DATA_R_H<<8) | PERI_DATA_R_L);
}

#ifdef _SPI_

void SpiWriteByte(BYTE chip_select, BYTE map, BYTE value)
{
	PERI_WriteByte(SPI_CHIP_SELECT, chip_select);
	PERI_WriteByte(SPI_DATA_0, map);
	PERI_WriteByte(SPI_DATA_1, value);
	PERI_WriteByte(SPI_CTRL_0, 0x02);
	PERI_WriteByte(SPI_CTRL_1, 0x80);

	while(PERI_ReadByte(SPI_CTRL_1) & bmBIT7);
}

BYTE SpiReadByte(BYTE chip_select, BYTE map)
{
	PERI_WriteByte(SPI_CHIP_SELECT, chip_select);
	PERI_WriteByte(SPI_DATA_0, map | bmBIT7);
	PERI_WriteByte(SPI_CTRL_0, 0x02);
	PERI_WriteByte(SPI_CTRL_1, 0x80);

	while(PERI_ReadByte(SPI_CTRL_1) & bmBIT7);

	return PERI_ReadByte(SPI_DATA_1);
}

#endif

#ifdef _I2C_MASTER_

// The maximum of len is 8.
// If the bit7 of paramter len is 1, I2C will run in 400k fast mode, else I2C runs in 100k standard mode.
// If the bit6 of paramter len is 1, I2C will turn on clock sync. mode.

BOOL I2cMasterWrite(BYTE addr, BYTE map, BYTE* buf, BYTE len)
{
	PERI_WriteByte(I2C_MASTER_DEST_ADDR, addr);
	PERI_WriteByte(I2C_MASTER_MAP_L, map);

	addr = bmBIT7;

	if(len & I2C_FAST_MODE)
		addr |= bmBIT3;

	if(len & I2C_CLOCK_SYNC)
		addr |= bmBIT4;

	len &= 0x0F;
	PERI_WriteByte(I2C_MASTER_CTRL_0, len);

	for(map = 0; map < len; ++map)
	{
		PERI_WriteByte(map + I2C_MASTER_BUFFER, buf[map]);
	}

	PERI_WriteByte(I2C_MASTER_CTRL_1, addr);

	while(PERI_ReadByte(I2C_MASTER_CTRL_1) & bmBIT7)
	{
		if(PERI_ReadByte(I2C_MASTER_CTRL_1) & bmBIT0)
		{
			PERI_WriteByte(I2C_MASTER_CTRL_1, bmBIT6);	// Reset I2C module
			return FALSE;
		}
	}

	return TRUE;
}

BOOL I2cMasterRead(BYTE addr, BYTE map, BYTE* buf, BYTE len)
{
	PERI_WriteByte(I2C_MASTER_DEST_ADDR, addr|0x01);
	PERI_WriteByte(I2C_MASTER_MAP_L, map);

	addr = bmBIT1 | bmBIT7;

	if(len & I2C_FAST_MODE)
		addr |= bmBIT3;

	if(len & I2C_CLOCK_SYNC)
		addr |= bmBIT4;

	len &= 0x0F;
	PERI_WriteByte(I2C_MASTER_CTRL_0, len);

	PERI_WriteByte(I2C_MASTER_CTRL_1, addr);

	while(PERI_ReadByte(I2C_MASTER_CTRL_1) & bmBIT7)
	{
		if(PERI_ReadByte(I2C_MASTER_CTRL_1) & bmBIT0)
		{
			PERI_WriteByte(I2C_MASTER_CTRL_1, bmBIT6);	// Reset I2C module
			return FALSE;
		}
	}

	for(map = 0; map < len; ++map)
	{
		buf[map] = PERI_ReadByte(map + I2C_MASTER_BUFFER);
	}

	return TRUE;
}

#endif

#ifdef _HDA_LINK_

void SendHdaCommand(BYTE node, WORD verb, WORD payload)
{
	#define HDA_ADDRESS 0x01

	PERI_ReadByte(HDA_VERB_RESPONSE_0);	// Clear response flag

	if(verb < 0x10)
	{
		PERI_WriteWord(HDA_VERB_RESPONSE_0, payload);
		PERI_WriteWord(HDA_VERB_RESPONSE_2, (HDA_ADDRESS<<12) | (node<<4) | verb);
	}
	else
	{
		PERI_WriteWord(HDA_VERB_RESPONSE_0, ((verb&0x00FF)<<8) | (payload&0x00FF));
		PERI_WriteWord(HDA_VERB_RESPONSE_2, (HDA_ADDRESS<<12) | (node<<4) | ((verb>>8)&0x000F));
	}

	for(verb = 0; verb < 200; ++verb)
	{
		node = PERI_ReadByte(HDA_STATUS_CTRL_H);
		if(node & bmBIT0)	// Check response flag
			break;
	}

	PERI_WriteByte(HDA_STATUS_CTRL_H, node & ~bmBIT3);
	PERI_ReadByte(HDA_VERB_RESPONSE_0);	// Clear response flag
}

#endif

BYTE FlashRead(WORD addr)
{
	return *((BYTE code *)addr);
}

