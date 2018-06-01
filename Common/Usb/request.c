#include "usb.h"

//-----------------------------------------------------------------------------
// Data Type Definition
//-----------------------------------------------------------------------------
typedef struct
{
   BYTE   index;
   BYTE   length;	// String descriptor length
   BYTE   type;	// Descriptor type
} STRING_DSCR;

typedef struct
{
	BYTE tag1;
	BYTE tag2;
	BYTE vid_pid[4];
	WORD config;
} CONFIG_TABLE;

//-----------------------------------------------------------------------------
// Customization Tables
//-----------------------------------------------------------------------------
CONFIG_TABLE code g_ConfigTable _at_ 0x1000;
STRING_DSCR code g_ConfigString _at_ 0x1010;
BOOL g_ConfigTableValid = FALSE;

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
BYTE idata g_UsbConfiguration;

//-----------------------------------------------------------------------------
// Static Variables
//-----------------------------------------------------------------------------
static USB_DEVICE code *s_pUsbTable;
static void *s_Ptr;

//-----------------------------------------------------------------------------
// External Variables
//-----------------------------------------------------------------------------
extern BOOL g_ReplaceOtherSpeed;
extern BOOL g_ReplaceVidPid;

//-----------------------------------------------------------------------------
// Code
//-----------------------------------------------------------------------------
static void GetStringDscr(BYTE index)
{
	if(g_ConfigTableValid)
	{
		s_Ptr = &g_ConfigString;

		while(((STRING_DSCR*)s_Ptr)->type == DSCR_TYPE_STRING)
		{
			if(((STRING_DSCR*)s_Ptr)->index == index)
				return;

			s_Ptr += (1+((STRING_DSCR*)s_Ptr)->length);
		}
	}

	s_Ptr = s_pUsbTable->pStringDscr;

	while(((STRING_DSCR*)s_Ptr)->type == DSCR_TYPE_STRING)
	{
		if(((STRING_DSCR*)s_Ptr)->index == index)
			return;

		s_Ptr += (1+((STRING_DSCR*)s_Ptr)->length);
	}
	
	s_Ptr = NULL;
}

static BOOL HandleGetDescriptor()
{
	switch(g_UsbRequest.value_H) 		
	{
		case GD_DEVICE:
			SetUsbCtrlData(s_pUsbTable->pDeviceDscr, DSCR_DEVICE_LEN);

			if(g_ConfigTableValid)
				g_ReplaceVidPid = TRUE;

			return TRUE;

		case GD_CONFIGURATION:
			SetUsbCtrlData(s_pUsbTable->pConfigDscr, (s_pUsbTable->pConfigDscr[3] << 8) | s_pUsbTable->pConfigDscr[2]);
			return TRUE;

		case GD_STRING:
			GetStringDscr(g_UsbRequest.value_L);
			if(!s_Ptr)
				return FALSE;

			SetUsbCtrlData(s_Ptr+1, ((STRING_DSCR*)s_Ptr)->length);
			return TRUE;

		case GD_DEVICE_QUALIFIER:
			if(s_pUsbTable->pDeviceQualDscr)
			{
				SetUsbCtrlData(s_pUsbTable->pDeviceQualDscr, DSCR_DEVQUAL_LEN);
				return TRUE;
			}
			else
			{
				return FALSE;
			}
	
		case GD_OTHER_SPEED_CONFIGURATION:
			if(s_pUsbTable->pOtherConfigDscr)
			{
				SetUsbCtrlData(s_pUsbTable->pOtherConfigDscr, (s_pUsbTable->pOtherConfigDscr[3] << 8) | s_pUsbTable->pOtherConfigDscr[2]);
				g_ReplaceOtherSpeed = TRUE;
				return TRUE;
			}
			else
			{
				return FALSE;
			}
	}

	return FALSE;
}

static BOOL HandleSetConfiguration()
{
	g_UsbConfiguration = g_UsbRequest.value_L;
	
	if(g_UsbConfiguration)
	{
		PERI_WriteByte(USB_CFG_RESET, 1);

		if(s_pUsbTable->Configured != NULL)
			s_pUsbTable->Configured();
	}
	else
	{
		PERI_WriteByte(USB_CFG_RESET, 0);

		if(s_pUsbTable->Deconfigured != NULL)
			s_pUsbTable->Deconfigured();
	}

	SetUsbCtrlData(g_UsbCtrlData, 0);
	return TRUE;
}

static BOOL HandleGetConfiguration()
{
	SetUsbCtrlData(&g_UsbConfiguration, 1);
	return TRUE;
}

static BOOL HandleSetInterface()
{
	if(g_UsbConfiguration && (g_UsbRequest.index_L < s_pUsbTable->interfaceNumbers))
	{
		s_Ptr = s_pUsbTable->pInterfaces[g_UsbRequest.index_L];

		if((((USB_INTERFACE*)s_Ptr)->pAltSetting) && (g_UsbRequest.value_L<=((USB_INTERFACE*)s_Ptr)->maxAltSettingNumber))
		{
			if(((USB_INTERFACE*)s_Ptr)->SetInterface == NULL)
			{
				*(((USB_INTERFACE*)s_Ptr)->pAltSetting) = g_UsbRequest.value_L;

				SetUsbCtrlData(g_UsbCtrlData, 0);
				return TRUE;
			}
			else if(((USB_INTERFACE*)s_Ptr)->SetInterface(s_Ptr))
			{
				*(((USB_INTERFACE*)s_Ptr)->pAltSetting) = g_UsbRequest.value_L;

				SetUsbCtrlData(g_UsbCtrlData, 0);
				return TRUE;
			}
		}
	}

	return FALSE;
}

static BOOL HandleGetInterface()
{
	if(g_UsbConfiguration && (g_UsbRequest.index_L < s_pUsbTable->interfaceNumbers))
	{
		s_Ptr = s_pUsbTable->pInterfaces[g_UsbRequest.index_L];

		if(((USB_INTERFACE*)s_Ptr)->pAltSetting)
		{
			SetUsbCtrlData(((USB_INTERFACE*)s_Ptr)->pAltSetting, 1);
		}
		else
		{
			g_UsbCtrlData[0] = 0;
			SetUsbCtrlData(g_UsbCtrlData, 1);
		}

		return TRUE;
	}

	return FALSE;
}

void RegisterUsbDevice(USB_DEVICE* device)
{
	s_pUsbTable = device;

	for(g_Index=0; g_Index<s_pUsbTable->interfaceNumbers; ++g_Index)
	{
		s_Ptr = s_pUsbTable->pInterfaces[g_Index];

		if(((USB_INTERFACE*)s_Ptr)->pAltSetting)
			*(((USB_INTERFACE*)s_Ptr)->pAltSetting) = 0;
	}
}

BOOL HandleRequest(BOOL data_out_stage)
{
	s_Ptr = NULL;

	if((g_UsbRequest.type & SETUP_MASK) == SETUP_STANDARD_REQUEST)
	{
		if(data_out_stage)
		{
			switch(g_UsbRequest.request)
			{
				case SC_SET_INTERFACE:
					return TRUE;
		
				case SC_SET_CONFIGURATION:
					return TRUE;
			}
		}
		else
		{
			switch(g_UsbRequest.request)
			{
				case SC_GET_DESCRIPTOR:
					if(((g_UsbRequest.type&RECIPIENT_MASK) == RECIPIENT_INTERFACE) && 
						(g_UsbRequest.index_L < s_pUsbTable->interfaceNumbers))
					{
						s_Ptr = s_pUsbTable->pInterfaces[g_UsbRequest.index_L];

						if(((USB_INTERFACE*)s_Ptr)->HandleGetDesc == NULL)
							return FALSE;
						else
							return ((USB_INTERFACE*)s_Ptr)->HandleGetDesc(s_Ptr);
					}
					else
					{
						return HandleGetDescriptor();
					}
		
				case SC_GET_INTERFACE:
					return HandleGetInterface();
					
				case SC_SET_INTERFACE:
					return HandleSetInterface();
		
				case SC_SET_CONFIGURATION:
					return HandleSetConfiguration();
					
				case SC_GET_CONFIGURATION:
					return HandleGetConfiguration();
			}
		}
	}
	else if((g_UsbRequest.type&SETUP_MASK) == SETUP_CLASS_REQUEST)
	{
		if(((g_UsbRequest.type&RECIPIENT_MASK) == RECIPIENT_INTERFACE) && 
			(g_UsbRequest.index_L < s_pUsbTable->interfaceNumbers))
		{
			s_Ptr = s_pUsbTable->pInterfaces[g_UsbRequest.index_L];

			if(data_out_stage)
			{
				if(((USB_INTERFACE*)s_Ptr)->HandleClassDataOut == NULL)
					return FALSE;
				else
					return ((USB_INTERFACE*)s_Ptr)->HandleClassDataOut(s_Ptr);
			}
			else
			{
				if(((USB_INTERFACE*)s_Ptr)->HandleClassCmnd == NULL)
					return FALSE;
				else
					return ((USB_INTERFACE*)s_Ptr)->HandleClassCmnd(s_Ptr);
			}
		}
		else if((g_UsbRequest.type&RECIPIENT_MASK) == RECIPIENT_ENDPOINT)
		{
			for(g_Index=0; g_Index<s_pUsbTable->interfaceNumbers; ++g_Index)
			{
				for(g_TempByte1=0; g_TempByte1<s_pUsbTable->pInterfaces[g_Index]->endpointNumbers; ++g_TempByte1)
				{
					if(g_UsbRequest.index_L == s_pUsbTable->pInterfaces[g_Index]->pEndpoints[g_TempByte1].number)
					{
						s_Ptr = &(s_pUsbTable->pInterfaces[g_Index]->pEndpoints[g_TempByte1]);
						break;
					}
				}

				if(s_Ptr)
					break;
			}

			if(!s_Ptr)
				return FALSE;

			if(data_out_stage)
			{
				if(((USB_ENDPOINT*)s_Ptr)->HandleClassDataOut == NULL)
					return FALSE;
				else
					return ((USB_ENDPOINT*)s_Ptr)->HandleClassDataOut(s_Ptr);
			}
			else
			{
				if(((USB_ENDPOINT*)s_Ptr)->HandleClassCmnd == NULL)
					return FALSE;
				else
					return ((USB_ENDPOINT*)s_Ptr)->HandleClassCmnd(s_Ptr);
			}
		}

		return FALSE;
	}
	else if((g_UsbRequest.type&SETUP_MASK) == SETUP_VENDOR_REQUEST)
	{
		if(data_out_stage)
		{
			if(s_pUsbTable->HandleVendorDataOut == NULL)
				return FALSE;
			else
				return s_pUsbTable->HandleVendorDataOut();
		}
		else
		{
			if(s_pUsbTable->HandleVendorCmnd == NULL)
				return FALSE;
			else
				return s_pUsbTable->HandleVendorCmnd();
		}
	}

	return FALSE;
}

