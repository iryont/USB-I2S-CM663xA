/*
 * Copyright (c) 2018-2021
 * Iryont <iryont@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include "audio.h"
#include "config.h"

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
enum
{
    AC_NODEF = 0,
    AC_IT,
    AC_OT,
    AC_FEATURE,
    AC_MIXER,
    AC_SELECTOR
};

enum
{
    CMD_NODEF = 0,
    CMD_SET_CURRENT,
    CMD_SET_MIN,
    CMD_SET_MAX,
    CMD_SET_RES,
    CMD_GET_CURRENT = 0x81,
    CMD_GET_MIN,
    CMD_GET_MAX,
    CMD_GET_RES
};

//-----------------------------------------------------------------------------
// USB Interface Structure Definition
//-----------------------------------------------------------------------------
BOOL StreamSetInterface();
BOOL HandleControlCmnd();
BOOL HandleControlDataOut();
BOOL HandleStreamCmnd();
BOOL HandleStreamDataOut();

USB_INTERFACE code g_Audio10InterfaceSpeaker;
USB_INTERFACE code g_Audio10InterfaceSpdifOut;
USB_INTERFACE code g_Audio10InterfaceHeadphone;
USB_INTERFACE code g_Audio10InterfaceLineIn;
USB_INTERFACE code g_Audio10InterfaceMicIn;
USB_INTERFACE code g_Audio10InterfaceSpdifIn;

static USB_ENDPOINT code s_SpeakerEndpoints[1] = 
{
	{
		EP_MULTI_CH_PLAY,
		&g_Audio10InterfaceSpeaker,

		HandleStreamCmnd,
		HandleStreamDataOut
	}
};

USB_INTERFACE code g_Audio10InterfaceAudioCtrl = 
{
	0,
	NULL,

	0,
	NULL, 

	NULL,
	NULL,

	HandleControlCmnd,
	HandleControlDataOut
};

extern BYTE idata g_AltSpeaker;
USB_INTERFACE code g_Audio10InterfaceSpeaker = 
{
	2,
	&g_AltSpeaker,

	1,
	s_SpeakerEndpoints,

	StreamSetInterface,
	NULL,

	NULL,
	NULL
};

//-----------------------------------------------------------------------------
// Static Variables
//-----------------------------------------------------------------------------
static BYTE code AudioUnitTable[] = 
{
	AC_NODEF,		// 0
	AC_IT,			// 1
	AC_IT,			// 2
	AC_IT,			// 3
	AC_IT,			// 4
	AC_IT,			// 5
	AC_IT,			// 6
	AC_OT,			// 7
	AC_OT,			// 8
	AC_OT,			// 9
	AC_OT,			// 10
	AC_OT,			// 11
	AC_OT,			// 12
	AC_FEATURE,		// 13
	AC_FEATURE,		// 14
	AC_FEATURE,		// 15
	AC_FEATURE,		// 16
	AC_FEATURE		// 17
};

//-----------------------------------------------------------------------------
// Code
//-----------------------------------------------------------------------------
static BOOL HandleVolume(BOOL data_out_stage)
{
	for(g_Index = 0; g_Index < VOLUME_DSCR_NUM; ++g_Index)
	{
		if((g_UsbRequest.index_H == g_VolumeTable[g_Index].id) && 
				(g_UsbRequest.value_L == g_VolumeTable[g_Index].ch))
			break;
	}
	
	if(VOLUME_DSCR_NUM == g_Index)
	{
		return FALSE;
	}

	switch(g_UsbRequest.request)
	{
		case CMD_GET_MIN:
			g_UsbCtrlData[0] = LSB(g_VolumeTable[g_Index].min);
			g_UsbCtrlData[1] = MSB(g_VolumeTable[g_Index].min);
			SetUsbCtrlData(g_UsbCtrlData, 2);
			return TRUE;

		case CMD_GET_MAX:
			g_UsbCtrlData[0] = LSB(g_VolumeTable[g_Index].max);
			g_UsbCtrlData[1] = MSB(g_VolumeTable[g_Index].max);
			SetUsbCtrlData(g_UsbCtrlData, 2);
			return TRUE;

		case CMD_GET_RES:
			g_UsbCtrlData[0] = LSB(g_VolumeTable[g_Index].res);
			g_UsbCtrlData[1] = MSB(g_VolumeTable[g_Index].res);
			SetUsbCtrlData(g_UsbCtrlData, 2);
			return TRUE;

		case CMD_GET_CURRENT:
			g_TempWord1 = GetCurrentVolume(g_Index);
			g_UsbCtrlData[0] = LSB(g_TempWord1);
			g_UsbCtrlData[1] = MSB(g_TempWord1);
			SetUsbCtrlData(g_UsbCtrlData, 2);
			return TRUE;

		case CMD_SET_CURRENT:
			if(data_out_stage)
			{
				SetCurrentVolume(g_Index, (g_UsbCtrlData[1] << 8) | g_UsbCtrlData[0]);
			}
			else
			{
				SetUsbCtrlData(g_UsbCtrlData, 2);
			}
			return TRUE;
	}

	return FALSE;
}

static BOOL HandleMute(BOOL data_out_stage)
{
	if((g_UsbRequest.value_L == 0) || (g_UsbRequest.value_L == 0xFF)) /* CN = 0 or 0xFF */
	{
		for(g_Index = 0; g_Index < MUTE_DSCR_NUM; ++g_Index)
		{
			if(g_UsbRequest.index_H == g_MuteTable[g_Index])
				break;			
		}

		if(MUTE_DSCR_NUM == g_Index)
		{
			return FALSE;
		}
	
		switch(g_UsbRequest.request)
		{
			case CMD_GET_CURRENT:
				g_UsbCtrlData[0] = GetCurrentMute(g_Index);
				SetUsbCtrlData(g_UsbCtrlData, 1);
				return TRUE;

			case CMD_SET_CURRENT:
				if(data_out_stage)
				{
					SetCurrentMute(g_Index, g_UsbCtrlData[0]);
				}
				else
				{
					SetUsbCtrlData(g_UsbCtrlData, 1);
				}
				return TRUE;
		}
	}
	
	return FALSE;
}

static BOOL HandleControlCmnd()
{
	switch(AudioUnitTable[g_UsbRequest.index_H])
	{
	case AC_FEATURE:
		if(g_UsbRequest.value_H == 1) /* Mute Control */
		{
			return HandleMute(FALSE);
		}
		else if(g_UsbRequest.value_H == 2) /* Volume Control */
		{
			return HandleVolume(FALSE);
		}	
	}

	return FALSE;
}

static BOOL HandleControlDataOut()
{
	switch(AudioUnitTable[g_UsbRequest.index_H])
	{
	case AC_FEATURE:
		if(g_UsbRequest.value_H == 1)      /* Mute Control */
		{
			return HandleMute(TRUE);
		}
		else if(g_UsbRequest.value_H == 2) /* Volume Control */
		{
			return HandleVolume(TRUE);
		}
	}
	
	return FALSE;
}

static BOOL PlayMultiChOpen(BYTE alt)
{
	switch(alt)
	{
		case 1: // 2ch 16bits
			return PlayMultiChStart(DMA_2CH, DMA_16Bit);

		case 2: // 2ch 24bits
			return PlayMultiChStart(DMA_2CH, DMA_24Bit);
	}

	return FALSE;
}

static BOOL StreamSetInterface(USB_INTERFACE* pInterface)
{
	if(!g_UsbRequest.value_L)
	{
		if(pInterface->endpointNumbers != 1)
			return FALSE;

		switch(pInterface->pEndpoints[0].number)
		{
		case EP_MULTI_CH_PLAY:
			return PlayMultiChStop();

		default:
			return FALSE;
		}
	}

	return TRUE;
}

static BOOL HandleStreamCmnd(USB_ENDPOINT* pEndpoint)
{
	// CS = sampling rate control && length = 3
	if((g_UsbRequest.value_L == 0) && (g_UsbRequest.value_H == 1) && (g_UsbCtrlLength == 3))
	{
		if(CMD_GET_CURRENT == g_UsbRequest.request)
		{
			g_TempByte1 = GetDmaFreq(pEndpoint->number);

			ControlByteToFreq(g_TempByte1);
			SetUsbCtrlData(g_UsbCtrlData, 3);
			return TRUE;
		}
		else if(CMD_SET_CURRENT == g_UsbRequest.request)
		{
			SetUsbCtrlData(g_UsbCtrlData, 3);
			return TRUE;
		}
	}
	
	return FALSE;
}

static BOOL HandleStreamDataOut(USB_ENDPOINT* pEndpoint)
{
	// CS = sampling rate control && length = 3
	if((g_UsbRequest.value_L == 0) && (g_UsbRequest.value_H == 1) && (g_UsbCtrlLength == 3))
	{
		if(CMD_SET_CURRENT == g_UsbRequest.request)
		{
			g_TempByte1 = *((USB_INTERFACE*)(pEndpoint->pInterface))->pAltSetting;

			switch(pEndpoint->number)
			{
			case EP_MULTI_CH_PLAY:
				SetDmaFreq(EP_MULTI_CH_PLAY, FreqToControlByte());
				return PlayMultiChOpen(g_TempByte1);

			default:
				return FALSE;
			}
		}
	}

	return FALSE;
}

