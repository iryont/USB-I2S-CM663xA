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

#ifndef _CODEC_H_
#define _CODEC_H_

#include "usb.h"
#include "config.h"

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define MUTE_DSCR_NUM 2
#define VOLUME_DSCR_NUM 2

//-----------------------------------------------------------------------------
// Type Definition
//-----------------------------------------------------------------------------
typedef struct
{
	BYTE   id;
	BYTE   ch;
	WORD   min;
	WORD   max;
	WORD   res;
	BYTE   type;
} VOLUME_DSCR;

//-----------------------------------------------------------------------------
// Public Variable
//-----------------------------------------------------------------------------
extern BOOL g_IsAudioClass20;
extern BOOL g_InputSourceSpdif;

extern BYTE code g_MuteTable[MUTE_DSCR_NUM];
extern VOLUME_DSCR code g_VolumeTable[VOLUME_DSCR_NUM];

extern BOOL g_SpdifLockStatus;
extern BYTE idata g_SpdifRateStatus;

extern USB_INTERFACE code g_Audio20InterfaceAudioCtrl;
extern USB_INTERFACE code g_Audio20InterfaceSpeaker;
extern USB_INTERFACE code g_Audio20InterfaceSpdifOut;
extern USB_INTERFACE code g_Audio20InterfaceSpdifIn;

extern USB_INTERFACE code g_Audio10InterfaceAudioCtrl;
extern USB_INTERFACE code g_Audio10InterfaceSpeaker;
extern USB_INTERFACE code g_Audio10InterfaceSpdifOut;
extern USB_INTERFACE code g_Audio10InterfaceSpdifIn;

//-----------------------------------------------------------------------------
// Public Function
//-----------------------------------------------------------------------------
extern void AudioInit();
extern void AudioStop();
extern void AudioProcess();

extern void ControlByteToFreq(BYTE value);
extern BYTE FreqToControlByte();

extern BYTE GetDmaFreq(BYTE endpoint);
extern void SetDmaFreq(BYTE endpoint, BYTE freq);

extern BOOL PlayMultiChStart(BYTE ch, BYTE format);
extern BOOL PlaySpdifStart(BYTE format);
extern BOOL RecSpdifStart(BYTE format);

extern BOOL PlayMultiChStop();
extern BOOL PlaySpdifStop();
extern BOOL RecSpdifStop();

extern BOOL GetCurrentMute(BYTE index);
extern void SetCurrentMute(BYTE index, BOOL mute);

extern WORD GetCurrentVolume(BYTE index);
extern void SetCurrentVolume(BYTE index, WORD volume);

#endif

