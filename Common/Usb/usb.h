#ifndef _USB_H_
#define _USB_H_

#include "common.h"

//-----------------------------------------------------------------------------
// Constants Definition
//-----------------------------------------------------------------------------
#define DSCR_TYPE_DEVICE      0x01      // Descriptor type: Device
#define DSCR_TYPE_CONFIG      0x02      // Descriptor type: Configuration
#define DSCR_TYPE_STRING      0x03      // Descriptor type: String
#define DSCR_TYPE_INTRFC      0x04      // Descriptor type: Interface
#define DSCR_TYPE_ENDPNT      0x05      // Descriptor type: End Point
#define DSCR_TYPE_DEVQUAL     0x06      // Descriptor type: Device Qualifier
#define DSCR_TYPE_OTHERSPEED  0x07      // Descriptor type: Other Speed Configuration

#define DSCR_DEVICE_LEN 18
#define DSCR_DEVQUAL_LEN 10
#define DSCR_HID_LEN 9  

#define SETUP_MASK				0x60	//Used to mask off request type
#define SETUP_STANDARD_REQUEST	0		//Standard Request
#define SETUP_CLASS_REQUEST		0x20	//Class Request
#define SETUP_VENDOR_REQUEST	0x40	//Vendor Request
#define SETUP_RESERVED_REQUEST 	0x60	//Reserved or illegal request

#define SC_GET_STATUS         0x00   // Setup command: Get Status
#define SC_CLEAR_FEATURE      0x01   // Setup command: Clear Feature
#define SC_RESERVED            0x02   // Setup command: Reserved
#define SC_SET_FEATURE         0x03   // Setup command: Set Feature
#define SC_SET_ADDRESS         0x05   // Setup command: Set Address
#define SC_GET_DESCRIPTOR      0x06   // Setup command: Get Descriptor
#define SC_SET_DESCRIPTOR      0x07   // Setup command: Set Descriptor
#define SC_GET_CONFIGURATION   0x08   // Setup command: Get Configuration
#define SC_SET_CONFIGURATION   0x09   // Setup command: Set Configuration
#define SC_GET_INTERFACE      0x0a   // Setup command: Get Interface
#define SC_SET_INTERFACE      0x0b   // Setup command: Set Interface
#define SC_SYNC_FRAME         0x0c   // Setup command: Sync Frame
#define SC_ANCHOR_LOAD         0xa0   // Setup command: Anchor load
   
#define GD_DEVICE          0x01  // Get descriptor: Device
#define GD_CONFIGURATION   0x02  // Get descriptor: Configuration
#define GD_STRING          0x03  // Get descriptor: String
#define GD_INTERFACE       0x04  // Get descriptor: Interface
#define GD_ENDPOINT        0x05  // Get descriptor: Endpoint
#define GD_DEVICE_QUALIFIER 0x06  // Get descriptor: Device Qualifier
#define GD_OTHER_SPEED_CONFIGURATION 0x07  // Get descriptor: Other Configuration
#define GD_INTERFACE_POWER 0x08  // Get descriptor: Interface Power
#define GD_HID	            0x21	// Get descriptor: HID
#define GD_REPORT	        0x22	// Get descriptor: Report

#define GS_DEVICE          0x80  // Get Status: Device
#define GS_INTERFACE       0x81  // Get Status: Interface
#define GS_ENDPOINT        0x82  // Get Status: End Point

#define FT_DEVICE          0x00  // Feature: Device
#define FT_ENDPOINT        0x02  // Feature: End Point

#define RECIPIENT_MASK 0x1F
#define RECIPIENT_DEVICE 0x00
#define RECIPIENT_INTERFACE 0x01
#define RECIPIENT_ENDPOINT 0x02
#define RECIPIENT_OTHER 0x03

#define USB_CTRL_BUF_SIZE 16

//-----------------------------------------------------------------------------
// Type Definition
//-----------------------------------------------------------------------------
typedef struct
{
	BYTE type;
	BYTE request;
	BYTE value_L;
	BYTE value_H;
	BYTE index_L;
	BYTE index_H;
	BYTE length_L;
	BYTE length_H;
} USB_REQUEST;

typedef struct
{
	BYTE number;
	void* pInterface;
	
	BOOL (*HandleClassCmnd)(void*);
	BOOL (*HandleClassDataOut)(void*);
} USB_ENDPOINT;

typedef struct
{
	BYTE maxAltSettingNumber;
	BYTE* pAltSetting;

	BYTE endpointNumbers;
	USB_ENDPOINT* pEndpoints;
	
	BOOL (*SetInterface)(void*);
	BOOL (*HandleGetDesc)(void*);

	BOOL (*HandleClassCmnd)(void*);
	BOOL (*HandleClassDataOut)(void*);
} USB_INTERFACE;

typedef struct {
	BYTE* pDeviceDscr;
	BYTE* pDeviceQualDscr;
	BYTE* pConfigDscr;
	BYTE* pOtherConfigDscr;
	BYTE* pStringDscr;

	BYTE interfaceNumbers;
	USB_INTERFACE** pInterfaces;

	void (*Configured)();
	void (*Deconfigured)();

	BOOL (*HandleVendorCmnd)();
	BOOL (*HandleVendorDataOut)();
} USB_DEVICE;

//-----------------------------------------------------------------------------
// Public Variable
//-----------------------------------------------------------------------------
extern USB_REQUEST g_UsbRequest;
extern WORD g_UsbCtrlLength;
extern BYTE g_UsbCtrlData[USB_CTRL_BUF_SIZE];

extern BOOL g_UsbIsHighSpeed;
extern volatile BOOL g_UsbIsActive;

extern BYTE idata g_UsbConfiguration;

//-----------------------------------------------------------------------------
// Public Function
//-----------------------------------------------------------------------------
extern void UsbInit();
extern void UsbProcess();

extern void RegisterUsbDevice(USB_DEVICE* device);

extern void SetUsbCtrlData(BYTE* buf, WORD len);

extern void SendInt4Data(BYTE* buf, BYTE len);
extern void SendInt15Data(BYTE* buf, BYTE len);

#endif

