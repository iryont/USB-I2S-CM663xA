#include "usb.h"

//-----------------------------------------------------------------------------
// Data Type Definition
//-----------------------------------------------------------------------------
typedef enum
{
	NONE_STAGE,
	SETUP_STAGE,
	DATA_IN_STAGE,
	DATA_OUT_STAGE,
	STATUS_STAGE
} CONTROL_STATE;

typedef struct
{
	BYTE tag1;
	BYTE tag2;
	BYTE vid_pid[4];
	WORD config;
} CONFIG_TABLE;

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
volatile BOOL g_UsbIsActive;
BOOL g_UsbIsHighSpeed;
USB_REQUEST g_UsbRequest;
WORD g_UsbCtrlLength;
BYTE g_UsbCtrlData[USB_CTRL_BUF_SIZE];

BOOL g_ReplaceOtherSpeed;
BOOL g_ReplaceVidPid;

//-----------------------------------------------------------------------------
// Static Variables
//-----------------------------------------------------------------------------
static CONTROL_STATE s_ControlState;
static BYTE s_ControlStatus;

static BYTE s_Round;

static BYTE* s_DataBuffer;
static WORD s_DataLength;

//-----------------------------------------------------------------------------
// External Variables
//-----------------------------------------------------------------------------
extern CONFIG_TABLE code g_ConfigTable;
extern BOOL g_ConfigTableValid;

extern volatile BOOL g_UsbSuspendRequest;
extern volatile BOOL g_UsbResumeRequest;
extern volatile BOOL g_UsbResetRequest;
extern volatile BOOL g_Interrupt4InComplete;
extern volatile BOOL g_Interrupt15InComplete;

//-----------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------
extern BOOL HandleRequest(BOOL data_out_stage);

extern void HandleUsbReset();
extern void HandleUsbResume();
extern void HandleUsbSuspend();
extern void HandleInt4Complete();
extern void HandleInt15Complete();

//-----------------------------------------------------------------------------
// Code
//-----------------------------------------------------------------------------
static void ISR_INT0() interrupt 0
{
	g_UsbIsActive = TRUE;
}

static void HandleControlTransfer()
{
//
// Check setup stage
//

	if(USB_HANDSHAKE1 & bmBIT0)
	{
		s_ControlState = SETUP_STAGE;

		for(g_Index=0; g_Index<8; ++g_Index)
		{
			((BYTE*)(&g_UsbRequest))[g_Index] = CONTROL_OUT_DATA[g_Index];
		}
		g_UsbCtrlLength = (g_UsbRequest.length_H << 8) | g_UsbRequest.length_L;

		g_ReplaceOtherSpeed = FALSE;
		g_ReplaceVidPid = FALSE;
		s_Round = 0;

		if(HandleRequest(FALSE))
		{
			s_ControlStatus = STATUS_ACK;
			s_DataLength = min(g_UsbCtrlLength, s_DataLength);
		
			USB_CONTROL_LEN_L = LSB(s_DataLength);
			USB_CONTROL_LEN_H = MSB(s_DataLength);
			USB_HANDSHAKE2 = bmBIT3;

			if(!s_DataLength)
				s_ControlState = STATUS_STAGE;
			else if(g_UsbRequest.type & bmBIT7) // Device to Host
				s_ControlState = DATA_IN_STAGE;
			else
				s_ControlState = DATA_OUT_STAGE;
		}
		else
		{
			USB_HANDSHAKE2 = (bmBIT2 | bmBIT3);
			s_ControlState = NONE_STAGE;
		}
	}

//
// Check data out stage
//

	if((s_ControlState==DATA_OUT_STAGE) && (USB_HANDSHAKE1&bmBIT1))
	{
		if(s_DataLength > 8)
			g_TempByte1 = 8;
		else
			g_TempByte1 = s_DataLength;

		for(g_Index=0; g_Index<g_TempByte1; ++g_Index)
		{
			s_DataBuffer[g_Index] = CONTROL_OUT_DATA[g_Index];
		}

		USB_HANDSHAKE2 = bmBIT3;
		s_DataLength -= g_TempByte1;
		s_DataBuffer += g_TempByte1;

		if(!s_DataLength)
		{
			if(HandleRequest(TRUE))
				s_ControlStatus = STATUS_ACK;
			else
				s_ControlStatus = STATUS_STALL;

			s_ControlState = STATUS_STAGE;
		}
	}

//
// Check data in stage
//

	if((s_ControlState==DATA_IN_STAGE) && (USB_HANDSHAKE1&bmBIT2))
	{
		if(s_DataLength > 8)
			g_TempByte1 = 8;
		else
			g_TempByte1 = s_DataLength;


		for(g_Index=0; g_Index<g_TempByte1; ++g_Index)
		{
			if(g_ReplaceVidPid && s_Round==1 && g_Index<4)
			{
				CONTROL_IN_DATA[g_Index] = g_ConfigTable.vid_pid[g_Index];
			}
			else if(g_ReplaceOtherSpeed && s_Round==0 && g_Index == 1)
			{
				CONTROL_IN_DATA[g_Index] = 0x07;	
			}
			else
			{
				CONTROL_IN_DATA[g_Index] = s_DataBuffer[g_Index];
			}
		}

		USB_HANDSHAKE2 = bmBIT3;
		s_DataLength -= g_TempByte1;
		s_DataBuffer += g_TempByte1;

		s_Round ++;

		if(!s_DataLength)
		{
			s_ControlStatus = STATUS_ACK;
			s_ControlState = STATUS_STAGE;
		}
	}

//
// Check status stage
//

	if((s_ControlState==STATUS_STAGE) && (USB_HANDSHAKE1&bmBIT3))
	{
		USB_HANDSHAKE2 = (s_ControlStatus | bmBIT3);
		s_ControlState = NONE_STAGE;
	}
}

void UsbInit()
{
	s_ControlState = NONE_STAGE;
	g_UsbIsActive = FALSE;
	g_UsbConfiguration = 0;

	if(g_ConfigTable.tag1=='C' && g_ConfigTable.tag2=='M')
		g_ConfigTableValid = TRUE;
	else
		g_ConfigTableValid = FALSE;
}

void UsbProcess()
{
	if(g_UsbIsActive)
	{
		HandleControlTransfer();

		if(PERI_ReadByte(USB_TEST_MODE) & bmBIT2)	// For IF "Receiver Sensitivity" test
		{
			PERI_WriteByte(REQ_REDIRECT_L, 0x23);
			USB_HANDSHAKE2 = (bmBIT2 | bmBIT3);
		}
	}

	if(g_UsbResetRequest)
	{
		g_UsbResetRequest = FALSE;

		s_ControlState = NONE_STAGE;		
		g_UsbIsActive = TRUE;
		g_UsbConfiguration = 0;

		if(MCU_EXIST_REPORT & bmBIT3)	// Full speed
		{
			g_UsbIsHighSpeed = FALSE;
		}
		else
		{
			g_UsbIsHighSpeed = TRUE;
		}	

		HandleUsbReset();
	}

	if(g_UsbResumeRequest)
	{
		g_UsbResumeRequest = FALSE;

		g_UsbIsActive = TRUE;
		HandleUsbResume();
	}

	if(g_UsbSuspendRequest)
	{
		g_UsbSuspendRequest = FALSE;

		g_UsbIsActive = FALSE;
		HandleUsbSuspend();
	}

	if(g_Interrupt4InComplete)
	{
		g_Interrupt4InComplete = FALSE;
		HandleInt4Complete();
	}

	if(g_Interrupt15InComplete)
	{
		g_Interrupt15InComplete = FALSE;
		HandleInt15Complete();
	}
}

void SetUsbCtrlData(BYTE* buf, WORD len)
{
	s_DataBuffer = buf;
	s_DataLength = len;	
}

void SendInt4Data(BYTE* buf, BYTE len)
{
	for(g_Index=0; g_Index<len; ++g_Index)
	{
		PERI_WriteByte(INT_EP4_BUFFER+g_Index, buf[g_Index]);
	}

	PERI_WriteByte(INT_EP4_CTRL, len | bmBIT5);
}

void SendInt15Data(BYTE* buf, BYTE len)
{
	for(g_Index=0; g_Index<len; ++g_Index)
	{
		PERI_WriteByte(INT_EP15_BUFFER+g_Index, buf[g_Index]);
	}

	PERI_WriteByte(INT_EP15_CTRL, len | bmBIT4);
}

