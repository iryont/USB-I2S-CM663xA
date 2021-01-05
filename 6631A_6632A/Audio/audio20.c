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
    AC_SELECTOR,
    AC_CLKSRC,
    AC_EXTENSION
};

enum 
{
    CMD_NODEF = 0,
    CMD_CURRENT,
    CMD_RANGE,
    CMD_MEMORY
};

//-----------------------------------------------------------------------------
// Data Type Definition
//-----------------------------------------------------------------------------
typedef struct
{
   BYTE   id;
   BYTE   channel_no;
   WORD   channel_cfg;
} TE_CLUSTER_DSCR;

typedef struct
{
   BYTE   id;
   USB_INTERFACE*   pInterface;
} CLOCK_SOURCE_DSCR;

//-----------------------------------------------------------------------------
// USB Interface Structure Definition
//-----------------------------------------------------------------------------
BOOL StreamSetInterface();
BOOL HandleControlCmnd();
BOOL HandleControlDataOut();
BOOL HandleStreamCmnd();
BOOL HandleStreamDataOut();

BOOL PlayMultiChOpen(BYTE alt);

USB_INTERFACE code g_Audio20InterfaceAudioCtrl;
USB_INTERFACE code g_Audio20InterfaceSpeaker;

static USB_ENDPOINT code s_AudioIntEndpoints[1] = 
{
	{
		EP_AUDIO_INT,
		&g_Audio20InterfaceAudioCtrl,

		NULL,
		NULL
	}
};

static USB_ENDPOINT code s_SpeakerEndpoints[1] = 
{
	{
		EP_MULTI_CH_PLAY,
		&g_Audio20InterfaceSpeaker,

		NULL,
		NULL
	}
};

USB_INTERFACE code g_Audio20InterfaceAudioCtrl = 
{
	0,
	NULL,

	1,
	s_AudioIntEndpoints, 

	NULL,
	NULL,

	HandleControlCmnd,
	HandleControlDataOut
};

BYTE idata g_AltSpeaker;
USB_INTERFACE code g_Audio20InterfaceSpeaker = 
{
#ifdef _DSD_
#ifdef _SUPPORT_32BIT_
	4,
#else
	3,
#endif
#else
#ifdef _SUPPORT_32BIT_
	3,
#else
	2,
#endif
#endif
	&g_AltSpeaker,

	1,
	s_SpeakerEndpoints,

	StreamSetInterface,
	NULL,

	HandleStreamCmnd,
	HandleStreamDataOut
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
	AC_FEATURE,		// 17
	AC_CLKSRC,		// 18
	AC_CLKSRC,		// 19
	AC_CLKSRC,		// 20
	AC_CLKSRC,		// 21
	AC_CLKSRC,		// 22
	AC_CLKSRC		// 23
};

#if defined(_SUPPORT_1536K_)
#define I2S_EHS_CLOSK_SOURCE_RANGE_SIZE 146
static BYTE code s_I2sExperimentalHighSpeedClockSourceRange[I2S_EHS_CLOSK_SOURCE_RANGE_SIZE] = 
{
	12,0,
//	0x40,0x1F,0x00,0,0x40,0x1F,0x00,0,0,0,0,0,	// 8000
//	0x80,0x3E,0x00,0,0x80,0x3E,0x00,0,0,0,0,0,	// 16000
//	0x00,0x7D,0x00,0,0x00,0x7D,0x00,0,0,0,0,0,	// 32000
	0x44,0xAC,0x00,0,0x44,0xAC,0x00,0,0,0,0,0,	// 44100
	0x80,0xBB,0x00,0,0x80,0xBB,0x00,0,0,0,0,0,	// 48000
//	0x00,0xFA,0x00,0,0x00,0xFA,0x00,0,0,0,0,0,	// 64000
	0x88,0x58,0x01,0,0x88,0x58,0x01,0,0,0,0,0,	// 88200
	0x00,0x77,0x01,0,0x00,0x77,0x01,0,0,0,0,0,	// 96000
	0x10,0xB1,0x02,0,0x10,0xB1,0x02,0,0,0,0,0,	// 176400
	0x00,0xEE,0x02,0,0x00,0xEE,0x02,0,0,0,0,0,	// 192000
	0x20,0x62,0x05,0,0x20,0x62,0x05,0,0,0,0,0,	// 352800
	0x00,0xDC,0x05,0,0x00,0xDC,0x05,0,0,0,0,0,	// 384000
	0x40,0xC4,0x0A,0,0x40,0xC4,0x0A,0,0,0,0,0,	// 705600
	0x00,0xB8,0x0B,0,0x00,0xB8,0x0B,0,0,0,0,0,	// 768000
	0x80,0x88,0x15,0,0x80,0x88,0x15,0,0,0,0,0,	// 1411200
	0x00,0x70,0x17,0,0x00,0x70,0x17,0,0,0,0,0	// 1536000
};
#endif
#if defined(_SUPPORT_768K_)
#define I2S_HS_CLOSK_SOURCE_RANGE_SIZE 122
static BYTE code s_I2sHighSpeedClockSourceRange[I2S_HS_CLOSK_SOURCE_RANGE_SIZE] = 
{
	10,0,
//	0x40,0x1F,0x00,0,0x40,0x1F,0x00,0,0,0,0,0,	// 8000
//	0x80,0x3E,0x00,0,0x80,0x3E,0x00,0,0,0,0,0,	// 16000
//	0x00,0x7D,0x00,0,0x00,0x7D,0x00,0,0,0,0,0,	// 32000
	0x44,0xAC,0x00,0,0x44,0xAC,0x00,0,0,0,0,0,	// 44100
	0x80,0xBB,0x00,0,0x80,0xBB,0x00,0,0,0,0,0,	// 48000
//	0x00,0xFA,0x00,0,0x00,0xFA,0x00,0,0,0,0,0,	// 64000
	0x88,0x58,0x01,0,0x88,0x58,0x01,0,0,0,0,0,	// 88200
	0x00,0x77,0x01,0,0x00,0x77,0x01,0,0,0,0,0,	// 96000
	0x10,0xB1,0x02,0,0x10,0xB1,0x02,0,0,0,0,0,	// 176400
	0x00,0xEE,0x02,0,0x00,0xEE,0x02,0,0,0,0,0,	// 192000
	0x20,0x62,0x05,0,0x20,0x62,0x05,0,0,0,0,0,	// 352800
	0x00,0xDC,0x05,0,0x00,0xDC,0x05,0,0,0,0,0,	// 384000
	0x40,0xC4,0x0A,0,0x40,0xC4,0x0A,0,0,0,0,0,	// 705600
	0x00,0xB8,0x0B,0,0x00,0xB8,0x0B,0,0,0,0,0	// 768000
};
#elif defined(_SUPPORT_384K_)
#define I2S_HS_CLOSK_SOURCE_RANGE_SIZE 98
static BYTE code s_I2sHighSpeedClockSourceRange[I2S_HS_CLOSK_SOURCE_RANGE_SIZE] = 
{
	8,0,
//	0x40,0x1F,0,0,0x40,0x1F,0,0,0,0,0,0,	// 8000
//	0x80,0x3E,0,0,0x80,0x3E,0,0,0,0,0,0,	// 16000
//	0x00,0x7D,0,0,0x00,0x7D,0,0,0,0,0,0,	// 32000
	0x44,0xAC,0,0,0x44,0xAC,0,0,0,0,0,0,	// 44100
	0x80,0xBB,0,0,0x80,0xBB,0,0,0,0,0,0,	// 48000
//	0x00,0xFA,0,0,0x00,0xFA,0,0,0,0,0,0,	// 64000
	0x88,0x58,1,0,0x88,0x58,1,0,0,0,0,0,	// 88200
	0x00,0x77,1,0,0x00,0x77,1,0,0,0,0,0,	// 96000
	0x10,0xB1,2,0,0x10,0xB1,2,0,0,0,0,0,	// 176400
	0x00,0xEE,2,0,0x00,0xEE,2,0,0,0,0,0,	// 192000
	0x20,0x62,5,0,0x20,0x62,5,0,0,0,0,0,	// 352800
	0x00,0xDC,5,0,0x00,0xDC,5,0,0,0,0,0		// 384000
};
#else
#define I2S_HS_CLOSK_SOURCE_RANGE_SIZE 74
static BYTE code s_I2sHighSpeedClockSourceRange[I2S_HS_CLOSK_SOURCE_RANGE_SIZE] = 
{
	6,0,
//	0x40,0x1F,0,0,0x40,0x1F,0,0,0,0,0,0,	// 8000
//	0x80,0x3E,0,0,0x80,0x3E,0,0,0,0,0,0,	// 16000
//	0x00,0x7D,0,0,0x00,0x7D,0,0,0,0,0,0,	// 32000
	0x44,0xAC,0,0,0x44,0xAC,0,0,0,0,0,0,	// 44100
	0x80,0xBB,0,0,0x80,0xBB,0,0,0,0,0,0,	// 48000
//	0x00,0xFA,0,0,0x00,0xFA,0,0,0,0,0,0,	// 64000
	0x88,0x58,1,0,0x88,0x58,1,0,0,0,0,0,	// 88200
	0x00,0x77,1,0,0x00,0x77,1,0,0,0,0,0,	// 96000
	0x10,0xB1,2,0,0x10,0xB1,2,0,0,0,0,0,	// 176400
	0x00,0xEE,2,0,0x00,0xEE,2,0,0,0,0,0		// 192000
};
#endif

#define FS_CLOSK_SOURCE_RANGE_SIZE 26
static BYTE code s_FullSpeedClockSourceRange[FS_CLOSK_SOURCE_RANGE_SIZE] = 
{
	2,0,
	0x44,0xAC,0,0,0x44,0xAC,0,0,0,0,0,0,
	0x80,0xBB,0,0,0x80,0xBB,0,0,0,0,0,0
};

#define CLOCK_SOURCE_DSCR_NUM 1
static CLOCK_SOURCE_DSCR code s_ClockSourceTable[CLOCK_SOURCE_DSCR_NUM] = 
{
	{18, &g_Audio20InterfaceSpeaker}
};

static BYTE idata s_CurrentClock[CLOCK_SOURCE_DSCR_NUM] = 
{
	DMA_48000
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
	
	if(g_UsbRequest.type & bmBIT7) // Device to Host, Get request
	{
		if(CMD_CURRENT == g_UsbRequest.request)
		{
			g_TempWord1 = GetCurrentVolume(g_Index);
			g_UsbCtrlData[0] = LSB(g_TempWord1);
			g_UsbCtrlData[1] = MSB(g_TempWord1);
			SetUsbCtrlData(g_UsbCtrlData, 2);
			return TRUE;
		}	  
		else if(CMD_RANGE == g_UsbRequest.request)
		{
			g_UsbCtrlData[0] = 1;
			g_UsbCtrlData[1] = 0;
			g_UsbCtrlData[2] = LSB(g_VolumeTable[g_Index].min);
			g_UsbCtrlData[3] = MSB(g_VolumeTable[g_Index].min);
			g_UsbCtrlData[4] = LSB(g_VolumeTable[g_Index].max);
			g_UsbCtrlData[5] = MSB(g_VolumeTable[g_Index].max);
			g_UsbCtrlData[6] = LSB(g_VolumeTable[g_Index].res);
			g_UsbCtrlData[7] = MSB(g_VolumeTable[g_Index].res);
			SetUsbCtrlData(g_UsbCtrlData, 8);
			return TRUE;
		}
	}
	else
	{
		if(CMD_CURRENT == g_UsbRequest.request)
		{
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
	}      
	
	return FALSE;
}

static BOOL HandleMute(BOOL data_out_stage)
{
	if((g_UsbRequest.value_L == 0) && (CMD_CURRENT == g_UsbRequest.request)) /* CN = 0 */
	{
		for(g_Index = 0; g_Index<MUTE_DSCR_NUM; ++g_Index)
		{
			if(g_UsbRequest.index_H == g_MuteTable[g_Index])
				break;
		}		
	
		if(MUTE_DSCR_NUM == g_Index)
		{
			return FALSE;
		}
	
		if(g_UsbRequest.type & bmBIT7) // Device to Host, Get request
		{
			g_UsbCtrlData[0] = GetCurrentMute(g_Index);
			SetUsbCtrlData(g_UsbCtrlData, 1);
			return TRUE;
		}
		else
		{
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

static BOOL HandleClockValid()
{
	/* Get request and CN = 0 */
	if((g_UsbRequest.type & bmBIT7) && (CMD_CURRENT == g_UsbRequest.request) && (g_UsbRequest.value_L == 0))
	{
		for(g_Index = 0; g_Index<CLOCK_SOURCE_DSCR_NUM; ++g_Index)
		{
			if(g_UsbRequest.index_H == s_ClockSourceTable[g_Index].id)
				break;
		}
		
		if(CLOCK_SOURCE_DSCR_NUM == g_Index)
		{
			return FALSE;
		}

		g_UsbCtrlData[0] = 1;
		SetUsbCtrlData(g_UsbCtrlData, 1);
		return TRUE;
	}

	return FALSE;
}

static BOOL HandleClockFrequency(BOOL data_out_stage)
{
	for(g_Index = 0; g_Index < CLOCK_SOURCE_DSCR_NUM; ++g_Index)
	{
		if(g_UsbRequest.index_H == s_ClockSourceTable[g_Index].id)
			break;
	}

	if(CLOCK_SOURCE_DSCR_NUM == g_Index)
	{
		return FALSE;
	}

	if(g_UsbRequest.type & bmBIT7) // Device to Host, Get request
	{
		if(CMD_CURRENT == g_UsbRequest.request)
		{
			g_TempByte1 = s_CurrentClock[g_Index];

			ControlByteToFreq(g_TempByte1);
			SetUsbCtrlData(g_UsbCtrlData, 4);
			return TRUE;
		}
		else if(CMD_RANGE == g_UsbRequest.request)
		{
			if(g_UsbIsHighSpeed)
			{
#ifdef _SUPPORT_1536K_
				// only alt-setting 1 of the interface does support up to 1536 kHz
				//if(*(s_ClockSourceTable[g_Index].pInterface->pAltSetting) == 0)
					SetUsbCtrlData(s_I2sExperimentalHighSpeedClockSourceRange, I2S_EHS_CLOSK_SOURCE_RANGE_SIZE);
				//else
				//	SetUsbCtrlData(s_I2sHighSpeedClockSourceRange, I2S_HS_CLOSK_SOURCE_RANGE_SIZE);
#else
				SetUsbCtrlData(s_I2sHighSpeedClockSourceRange, I2S_HS_CLOSK_SOURCE_RANGE_SIZE);
#endif
			}
			else
			{
				SetUsbCtrlData(s_FullSpeedClockSourceRange, FS_CLOSK_SOURCE_RANGE_SIZE);
			}
			return TRUE;
		}	  
	}
	else /* Host to Device, Set request */
	{
		if(CMD_CURRENT == g_UsbRequest.request)
		{
			if(data_out_stage)
			{
 				g_TempByte1 = FreqToControlByte();
				s_CurrentClock[g_Index] = g_TempByte1;

				// If alt-setting of the interface is not 0, restart the stream.
				if(*(s_ClockSourceTable[g_Index].pInterface->pAltSetting))
				{
					switch(s_ClockSourceTable[g_Index].pInterface->pEndpoints[0].number)
					{
						case EP_MULTI_CH_PLAY:
							PlayMultiChStop();
							PlayMultiChOpen(*(s_ClockSourceTable[g_Index].pInterface->pAltSetting));
							break;
					}
				}
			}
			else
			{
				SetUsbCtrlData(g_UsbCtrlData, 4);
			}

			return TRUE;
		}
	}

	return FALSE;
}

static BOOL PlayMultiChOpen(BYTE alt)
{
	for(g_Index = 0; g_Index < CLOCK_SOURCE_DSCR_NUM; ++g_Index)
	{
		if(EP_MULTI_CH_PLAY == s_ClockSourceTable[g_Index].pInterface->pEndpoints[0].number)
			break;
	}

	if(CLOCK_SOURCE_DSCR_NUM == g_Index)
	{
		return FALSE;
	}

	SetDmaFreq(EP_MULTI_CH_PLAY, s_CurrentClock[g_Index]);

	switch(alt)
	{
		case 1: // 2ch 16bits
			return PlayMultiChStart(DMA_2CH, DMA_16Bit);

		case 2: // 2ch 24bits
			return PlayMultiChStart(DMA_2CH, DMA_24Bit);

#ifdef _SUPPORT_32BIT_
		case 3: // 2ch 32bits
			return PlayMultiChStart(DMA_2CH, DMA_32Bit);
#endif

#ifdef _DSD_
#ifdef _SUPPORT_32BIT_
		case 4: // 4ch 32bits
			return PlayMultiChStart(DMA_4CH, DMA_32Bit);
#else
		case 3: // 4ch 32bits
			return PlayMultiChStart(DMA_4CH, DMA_32Bit);
#endif
#endif
	}

	return FALSE;
}

static BOOL HandleControlCmnd()
{
	switch(AudioUnitTable[g_UsbRequest.index_H])
	{
	case AC_FEATURE:
		if(g_UsbRequest.value_H == 1) // Mute Control
		{
			return HandleMute(FALSE);
		}
		else if(g_UsbRequest.value_H == 2) // Volume Control
		{
			return HandleVolume(FALSE);
		}
		else if(g_UsbRequest.value_H == 7) // Auto Gain Control 
		{
			return FALSE;
		}
	  	break;

	case AC_CLKSRC:
		if(g_UsbRequest.value_H == 1) // Frequency Control
		{
			return HandleClockFrequency(FALSE);
		}
		else if(g_UsbRequest.value_H == 2) // Valid Control
		{
			return HandleClockValid();
		}
	  	break;
	}
 
	return FALSE;
}

static BOOL HandleControlDataOut()
{
	switch(AudioUnitTable[g_UsbRequest.index_H])
	{
	case AC_FEATURE:
		if(g_UsbRequest.value_H == 1) // Mute Control
		{
			return HandleMute(TRUE);
		}
		else if(g_UsbRequest.value_H == 2) // Volume Control
		{
			return HandleVolume(TRUE);
		}
		else if(g_UsbRequest.value_H == 7) // Auto Gain Control 
		{
			return FALSE;
		}	
	  	break;

	case AC_CLKSRC:
		if(g_UsbRequest.value_H == 1) // Frequency Control
		{
			return HandleClockFrequency(TRUE);
		}
	  	break;
	}

	return FALSE;
}

static BOOL StreamSetInterface(USB_INTERFACE* pInterface)
{
	if(pInterface->endpointNumbers != 1)
		return FALSE;

	if(g_UsbRequest.value_L)
	{
		switch(pInterface->pEndpoints[0].number)
		{
		case EP_MULTI_CH_PLAY:
			return PlayMultiChOpen(g_UsbRequest.value_L);

		default:
			return FALSE;
		}
	}
	else
	{
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

static BOOL HandleStreamCmnd(USB_INTERFACE* pInterface)
{
	if(pInterface->endpointNumbers != 1)
		return FALSE;

	/* Get CUR request and CN = 0, ID = 0 */
	if((CMD_CURRENT == g_UsbRequest.request) && (g_UsbRequest.value_L == 0) && (g_UsbRequest.index_H == 0))
	{
		switch(g_UsbRequest.value_H)
		{
			case 1:	// Active setting Control
				if(pInterface->pAltSetting)
				{
					SetUsbCtrlData(pInterface->pAltSetting, 1);
					return TRUE;
				}
	
			case 2:	// Valid setting Control
				g_TempByte1 = pInterface->maxAltSettingNumber;
				g_TempWord1 = 0;

				for(g_Index = 0; g_Index <= g_TempByte1; ++g_Index)
				{
					g_TempWord1 |= (1<<g_Index);
				}

				g_UsbCtrlData[0] = 2;
				g_UsbCtrlData[1] = LSB(g_TempWord1);
				g_UsbCtrlData[2] = MSB(g_TempWord1);

				SetUsbCtrlData(g_UsbCtrlData, 3);
				return TRUE;
	
			case 3:	// Data Format Control
				g_UsbCtrlData[0] = 1;
				g_UsbCtrlData[1] = 0;
				g_UsbCtrlData[2] = 0;
				g_UsbCtrlData[3] = 0;
	
				SetUsbCtrlData(g_UsbCtrlData, 4);
				return TRUE;
		}
	}

	return FALSE;
}

static BOOL HandleStreamDataOut()
{
	return FALSE;
}

