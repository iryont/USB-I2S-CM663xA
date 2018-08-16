#ifndef _CODEC_H_
#define _CODEC_H_

#include "usb.h"
#include "config.h"

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define MUTE_DSCR_NUM 1
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

extern BYTE code g_MuteTable[MUTE_DSCR_NUM];
extern VOLUME_DSCR code g_VolumeTable[VOLUME_DSCR_NUM];

extern USB_INTERFACE code g_Audio20InterfaceAudioCtrl;
extern USB_INTERFACE code g_Audio20InterfaceSpeaker;
//extern USB_INTERFACE code g_Audio20InterfaceSpdifOut;
//extern USB_INTERFACE code g_Audio20InterfaceHeadphone;
//extern USB_INTERFACE code g_Audio20InterfaceLineIn;
//extern USB_INTERFACE code g_Audio20InterfaceMicIn;
//extern USB_INTERFACE code g_Audio20InterfaceSpdifIn;

extern USB_INTERFACE code g_Audio10InterfaceAudioCtrl;
extern USB_INTERFACE code g_Audio10InterfaceSpeaker;
//extern USB_INTERFACE code g_Audio10Interface ifOut;
//extern USB_INTERFACE code g_Audio10InterfaceHeadphone;
//extern USB_INTERFACE code g_Audio10InterfaceLineIn;
//extern USB_INTERFACE code g_Audio10InterfaceMicIn;
//extern USB_INTERFACE code g_Audio10InterfaceSpdifIn;

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
extern BOOL PlayMultiChStop();

extern BOOL GetCurrentMute(BYTE index);
extern void SetCurrentMute(BYTE index, BOOL mute);

extern WORD GetCurrentVolume(BYTE index);
extern void SetCurrentVolume(BYTE index, WORD volume);

#endif

