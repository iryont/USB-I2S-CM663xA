#ifndef _CODEC_H_
#define _CODEC_H_

#include "usb.h"
#include "config.h"

//-----------------------------------------------------------------------------
// Public Variable
//-----------------------------------------------------------------------------
extern BOOL g_IsAudioClass20;

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

#endif

