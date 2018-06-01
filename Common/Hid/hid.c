#include "hid.h"

//-----------------------------------------------------------------------------
// External Variables
//-----------------------------------------------------------------------------
extern BYTE code HidDscr;
extern BYTE code HidReportDscr;
extern BYTE code HidReportDscrLen;

//-----------------------------------------------------------------------------
// Static Variables
//-----------------------------------------------------------------------------
typedef struct
{
	WORD addr;
	BYTE len;
} REG_INFO;

static REG_INFO idata s_HidInRegInfo;

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
HID_INPUT_REPORT idata g_InputReport;
BUFFER_16BYTES idata g_Buffer16Bytes;

//-----------------------------------------------------------------------------
// USB Interface Structure Definition
//-----------------------------------------------------------------------------
BOOL HandleHidGetDesc();
BOOL HandleHidCmnd();
BOOL HandleHidDataOut();

USB_INTERFACE code g_InterfaceHid;

static USB_ENDPOINT code s_HidEndpoints[1] = 
{
	{
		EP_HID_INT,
		&g_InterfaceHid,

		NULL,
		NULL
	}
};

USB_INTERFACE code g_InterfaceHid = 
{
	0,
	NULL,

	1,
	s_HidEndpoints,

	NULL,
	HandleHidGetDesc,

	HandleHidCmnd,
	HandleHidDataOut
};

//-----------------------------------------------------------------------------
// Code
//-----------------------------------------------------------------------------
void HidInit()
{
	for(g_Index=0; g_Index<INPUT_REPORT_SIZE; ++g_Index)
	{
		((BYTE*)(&g_InputReport))[g_Index] = 0;
	}

	for(g_Index=0; g_Index<OUTPUT_REPORT_SIZE; ++g_Index)
	{
		g_OutputReport[g_Index] = 0;
	}

	s_HidInRegInfo.addr = 0x00B0;
	s_HidInRegInfo.len = 4;
}

static void PackInputReport()
{
	g_InputReport.offset_high = MSB(s_HidInRegInfo.addr);
	g_InputReport.offset_low = LSB(s_HidInRegInfo.addr);
	g_InputReport.length = s_HidInRegInfo.len;

	for(g_Index = 0; g_Index < s_HidInRegInfo.len; g_Index++)
	{
		g_InputReport.registers[g_Index] = PERI_ReadByte(s_HidInRegInfo.addr + g_Index); 	
	}		
}

void SendInputReport()
{
	PackInputReport();
	SendInt4Data((BYTE*)(&g_InputReport), INPUT_REPORT_SIZE);
}

static BOOL HandleHidGetDesc()
{
	switch(g_UsbRequest.value_H) 		
	{
		case GD_REPORT:
			SetUsbCtrlData(&HidReportDscr, HidReportDscrLen);
			return TRUE;

		case GD_HID:
			SetUsbCtrlData(&HidDscr, DSCR_HID_LEN);
			return TRUE;
	}
}

static BOOL HandleHidCmnd()
{
	switch(g_UsbRequest.request)
	{
	case HID_GET_REPORT:
		if(g_UsbRequest.value_H == REPORT_TYPE_INPUT)
		{
			PackInputReport();
			SetUsbCtrlData((BYTE*)(&g_InputReport), INPUT_REPORT_SIZE);
			return TRUE;
		}
		else if(g_UsbRequest.value_H == REPORT_TYPE_OUTPUT)
		{
			SetUsbCtrlData(g_OutputReport, OUTPUT_REPORT_SIZE);
			return TRUE;
		}

		break;

	case HID_SET_REPORT:
		if(g_UsbRequest.value_H == REPORT_TYPE_INPUT)
		{
			SetUsbCtrlData((BYTE*)(&g_InputReport), INPUT_REPORT_SIZE);
			return TRUE;
		}
		else if(g_UsbRequest.value_H == REPORT_TYPE_OUTPUT)
		{
			SetUsbCtrlData(g_OutputReport, OUTPUT_REPORT_SIZE);
			return TRUE;
		}

		break;

	case HID_SET_IDLE:
		SetUsbCtrlData(g_UsbCtrlData, 0);
		return TRUE;
	}

	return FALSE;
 }

static BOOL HandleHidDataOut()
{
	switch(g_UsbRequest.request)
	{
	case HID_SET_REPORT:
		if(g_UsbRequest.value_H == REPORT_TYPE_INPUT)
		{
			return TRUE;
		}
		else if(g_UsbRequest.value_H == REPORT_TYPE_OUTPUT)
		{
			g_TempWord1 = (g_OutputReport[4] << 8) | g_OutputReport[5];
			g_OutputReport[6] = min(g_OutputReport[6], (OUTPUT_REPORT_SIZE - 7));

			if(g_TempWord1 == 0x00FF)
			{
				s_HidInRegInfo.addr = (g_OutputReport[7] << 8) | g_OutputReport[8];
				s_HidInRegInfo.len = g_OutputReport[6];	
			}	
			else
			{
				for(g_Index = 0; g_Index < g_OutputReport[6]; g_Index++)
				{
					PERI_WriteByte(g_TempWord1 + g_Index, g_OutputReport[7 + g_Index]);
				}				
			}

			return TRUE;
		}

		break;

	case HID_SET_IDLE:
		return TRUE;
	}

	return FALSE;
}

