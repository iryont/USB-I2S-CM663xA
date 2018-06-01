#ifndef _HID_H_
#define _HID_H_

#include "usb.h"

//-----------------------------------------------------------------------------
// Macro Definition
//-----------------------------------------------------------------------------
#define HID_GET_REPORT 0x01
#define HID_GET_IDLE 0x02
#define HID_GET_PROTOCOL 0x03
#define HID_SET_REPORT 0x09
#define HID_SET_IDLE 0x0A
#define HID_SET_PROTOCOL 0x0B

#define REPORT_TYPE_INPUT 1
#define REPORT_TYPE_OUTPUT 2
#define REPORT_TYPE_FEATURE 3

//-----------------------------------------------------------------------------
// Public Variable
//-----------------------------------------------------------------------------
typedef struct
{
	BYTE button;
	BYTE reserved1;
	BYTE reserved2;
	BYTE source;
	BYTE offset_high;
	BYTE offset_low;
	BYTE length;
	BYTE registers[9];
} HID_INPUT_REPORT;

#define INPUT_REPORT_SIZE 16
#define OUTPUT_REPORT_SIZE 16

#define g_OutputReport (g_Buffer16Bytes._byte)
extern HID_INPUT_REPORT idata g_InputReport;

extern USB_INTERFACE code g_InterfaceHid;

//-----------------------------------------------------------------------------
// Public Function
//-----------------------------------------------------------------------------
extern void HidInit();
extern void SendInputReport();

#endif

