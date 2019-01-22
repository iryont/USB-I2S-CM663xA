#include "audio.h"
#include "timer.h"
#include "peripheral.h"
#include "usb.h"
//#include "hid.h"
#include "config.h"

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define PLAYBACK_ROUTING_2CH 0xE4
#define PLAYBACK_ROUTING_4CH 0xD8
#define PLAYBACK_ROUTING_6CH 0xE4
#define PLAYBACK_ROUTING_8CH 0xE4

#define IS_PLAYING(index) (s_bPlaybackStartCount & (1 << index))

enum
{
	VOL_DAC = 0,
	VOL_DUMMY
};

enum	// DMA ID
{
	PLAY_8CH,
	PLAY_SPDIF,
	REC_SPDIF
};

enum	// DSD
{
	DSD_OFF,
	DSD_ON,
	DSD_RUNNING,
	DSD_STOPPING
};

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
BOOL g_IsAudioClass20;
BOOL g_InputSourceSpdif = FALSE;

//-----------------------------------------------------------------------------
// Static Variables
//-----------------------------------------------------------------------------
static BYTE s_bPlaybackStartCount;

#define SAMPLING_RATE_NUM 16
static BYTE code s_SamplingRateTable[SAMPLING_RATE_NUM][5] =
{
	{0x00, 0x7D, 0x00, DMA_32000, 32},
	{0x00, 0xAC, 0x44, DMA_44100, 44},
	{0x00, 0xBB, 0x80, DMA_48000, 48},
	{0x00, 0xFA, 0x00, DMA_64000, 64},
	{0x01, 0x58, 0x88, DMA_88200, 88},
	{0x01, 0x77, 0x00, DMA_96000, 96},
	{0x02, 0xB1, 0x10, DMA_176400, 176},
	{0x02, 0xEE, 0x00, DMA_192000, 192},
	{0x00, 0x1F, 0x40, DMA_8000, 8},
	{0x00, 0x3E, 0x80, DMA_16000, 16},
	{0x05, 0x62, 0x20, DMA_352800, 352},
	{0x05, 0xDC, 0x00, DMA_384000, 384},
	{0x0A, 0xC4, 0x40, DMA_705600, 706},
	{0x0B, 0xB8, 0x00, DMA_768000, 768},
	{0x15, 0x88, 0x80, DMA_1411200, 1411},
	{0x17, 0x70, 0x00, DMA_1536000, 1536}
};

VOLUME_DSCR code g_VolumeTable[VOLUME_DSCR_NUM] = 
{
	{13,1,0x8800,0,0x0080,VOL_DUMMY},{13,2,0x8800,0,0x0080,VOL_DUMMY}  // -120db ~ 0db, step 0.5 db
};

static WORD idata s_CurrentVolume[VOLUME_DSCR_NUM] = 
{
	0, 0	// 0dB
};

static BOOL s_Run768K = FALSE;
static BOOL s_Run1536K = FALSE;

BYTE code g_MuteTable[MUTE_DSCR_NUM] = {13};
static BYTE idata s_CurrentMute = FALSE;
static BOOL s_RecordMute = FALSE;

BOOL g_SpdifLockStatus = FALSE;
BYTE idata g_SpdifRateStatus = DMA_48000;

#ifdef _MCU_FEEDBACK_

typedef struct
{
	WORD mid;
	BYTE end;
} FEEDBACK;


// double exp = 32 ms (uframe) * Hz / 1000
// int mid = (int)exp
// int end = (exp - mid) * 2^8
static FEEDBACK code s_FeedbackTable[SAMPLING_RATE_NUM] =
{
	{0x0400, 0x00},	// 32k
	{0x0583, 0x33},	// 44.1k
	{0x0600, 0x00},	// 48k
	{0x0800, 0x00},	// 64k
	{0x0B06, 0x66},	// 88.2k
	{0x0C00, 0x00},	// 96k
	{0x160C, 0xCD},	// 176.4k
	{0x1800, 0x00},	// 192k
	{0x0100, 0x00},	// 8k
	{0x0200, 0x00},	// 16k
	{0x2C19, 0x9A},	// 352.8k
	{0x3000, 0x00},	// 384k
	{0x5833, 0x34},	// 705.6k
	{0x6000, 0x00},	// 768k
	{0xB066, 0x66},	// 1411.2k
	{0xC000, 0x00}	// 1536k
};

static WORD s_MultiChCount, s_SpdifCount;
static WORD s_MultiChCountOld, s_SpdifCountOld;
static WORD idata s_MultiChThreshold, s_SpdifThreshold;
static BYTE idata s_MultiChFeedbackRatio, s_SpdifFeedbackRatio;
static BOOL s_MultiChFeedbackStart, s_SpdifFeedbackStart;

#ifdef _DSD_
static BYTE s_DsdState = DSD_OFF;
#endif

static BYTE code s_uFrameThresholdTable[80] = 
{
//	32	44	48	64	88	96	176	192	8	16
	192,	176,	192,	192,	176,	192,	176,	192,	192,	192,	// 2ch 16bit
	192,	176,	192,	192,	176,	192,	176,	192,	192,	192,	// 2ch 24/32bit

	192,	176,	192,	192,	176,	192,	176,	192,	192,	192,	// 4ch 16bit
	192,	176,	192,	192,	176,	192,	176,	192,	192,	192,	// 4ch 24/32bit

	144,	132,	144,	144,	132,	144,	132,	144,	144,	144,	// 6ch 16bit
	144,	132,	144,	144,	132,	144,	132,	144,	144,	144,	// 6ch 24/32bit

	192,	176,	192,	192,	176,	192,	176,	192,	192,	192,	// 8ch 16bit
	192,	176,	192,	192,	176,	192,	176,	192,	192,	192	// 8ch 24/32bit
};

static BYTE code s_mSecondThresholdTable[80] = 
{
//	32	44	48	64	88	96	176	192	8	16
	192,	176,	192,	192,	176,	192,	176,	192,	192,	192,	// 2ch 16bit
	192,	176,	192,	192,	176,	192,	176,	192,	192,	192,	// 2ch 24/32bit

	192,	176,	192,	192,	176,	192,	176,	192,	192,	192,	// 4ch 16bit
	192,	176,	192,	192,	176,	192,	176,	192,	192,	192,	// 4ch 24/32bit

	144,	132,	144,	144,	132,	144,	132,	144,	144,	144,	// 6ch 16bit
	144,	132,	144,	144,	132,	144,	132,	144,	144,	144,	// 6ch 24/32bit

	192,	176,	192,	192,	176,	192,	176,	192,	192,	192,	// 8ch 16bit
	192,	176,	192,	192,	176,	192,	176,	192,	192,	192	// 8ch 24/32bit
};

#elif defined(_FEEDBACK_)

static BYTE code s_uFrameThresholdTable[80] = 
{
//	32	44	48	64	88	96	176	192	8	16
	208,	208,	208,	208,	208,	208,	208,	208,	208,	208,	// 2ch 16bit
	208,	208,	208,	208,	208,	208,	208,	208,	208,	208,	// 2ch 24/32bit

	208,	208,	208,	208,	208,	208,	208,	208,	208,	208,	// 4ch 16bit
	208,	208,	208,	208,	208,	208,	208,	208,	208,	208,	// 4ch 24/32bit

	208,	208,	208,	208,	208,	208,	208,	208,	208,	208,	// 6ch 16bit
	208,	208,	208,	208,	208,	208,	208,	208,	208,	208,	// 6ch 24/32bit

	208,	208,	208,	208,	208,	208,	208,	208,	208,	208,	// 8ch 16bit
	208,	208,	208,	208,	208,	208,	208,	208,	208,	208	// 8ch 24/32bit
};

static BYTE code s_mSecondThresholdTable[80] = 
{
//	32	44	48	64	88	96	176	192	8	16
	208,	208,	208,	208,	208,	208,	208,	208,	208,	208,	// 2ch 16bit
	208,	208,	208,	208,	208,	208,	208,	208,	208,	208,	// 2ch 24/32bit

	208,	208,	208,	208,	208,	208,	208,	208,	208,	208,	// 4ch 16bit
	208,	208,	208,	208,	208,	208,	208,	208,	208,	208,	// 4ch 24/32bit

	208,	208,	208,	208,	208,	208,	208,	208,	208,	208,	// 6ch 16bit
	208,	208,	208,	208,	208,	208,	208,	208,	208,	208,	// 6ch 24/32bit

	208,	208,	208,	208,	208,	208,	208,	208,	208,	208,	// 8ch 16bit
	208,	208,	208,	208,	208,	208,	208,	208,	208,	208	// 8ch 24/32bit
};

#else

static BYTE code s_uFrameThresholdTable[80] = 
{
//	32	44	48	64	88	96	176	192	8	16
	16,	22,	24,	32,	44,	48,	88,	96,	0,	0,	// 2ch 16bit
	32,	44,	48,	64,	88,	96,	176,	192,	0,	0,	// 2ch 24/32bit

	32,	44,	48,	64,	88,	96,	176,	192,	0,	0,	// 4ch 16bit
	64,	88,	96,	128,	176,	192,	176,	192,	0,	0,	// 4ch 24/32bit

	48,	66,	72,	96,	132,	144,	132,	144,	0,	0,	// 6ch 16bit
	96,	132,	144,	96,	132,	144,	132,	144,	0,	0,	// 6ch 24/32bit

	64,	88,	96,	128,	176,	192,	176,	192,	0,	0,	// 8ch 16bit
	128,	176,	192,	128,	176,	192,	176,	192,	0,	0	// 8ch 24/32bit
};

static BYTE code s_mSecondThresholdTable[80] = 
{
//	32	44	48	64	88	96	176	192	8	16
	64,	88,	96,	128,	176,	192,	0,	0,	64,	64,	// 2ch 16bit
	128,	176,	192,	144,	192,	208,	0,	0,	128,	128,	// 2ch 24/32bit

	128,	176,	192,	144,	192,	208,	0,	0,	128,	128,	// 4ch 16bit
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	// 4ch 24/32bit

	192,	148,	160,	0,	0,	0,	0,	0,	192,	192,	// 6ch 16bit
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	// 6ch 24/32bit

	144,	192,	208,	0,	0,	0,	0,	0,	192,	192,	// 8ch 16bit
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0	// 8ch 24/32bit
};

#endif

//-----------------------------------------------------------------------------
// Code
//-----------------------------------------------------------------------------
void SetPinFreq(BYTE value)
{
	BYTE gpio = PERI_ReadByte(GPIO_DATA_H);

	if(value & bmBIT0)
		PERI_WriteByte(PAD_GP12_GP13_CTRL, PERI_ReadByte(PAD_GP12_GP13_CTRL) & ~bmBIT3); // enable 44.1
	else
		PERI_WriteByte(PAD_GP12_GP13_CTRL, PERI_ReadByte(PAD_GP12_GP13_CTRL) | bmBIT3); // disable 44.1

	if(value & bmBIT3) // F3
		gpio |= bmBIT0;
	else
		gpio &= ~bmBIT0;

	if(value & bmBIT2) // F2
		gpio |= bmBIT2;
	else
		gpio &= ~bmBIT2;

	if(value & bmBIT1) // F1
		gpio |= bmBIT4;
	else
		gpio &= ~bmBIT4;

	PERI_WriteByte(GPIO_DATA_H, gpio);
}

void ControlByteToFreq(BYTE value)
{
	BYTE index;

	for(index = 0; index<SAMPLING_RATE_NUM; ++index)	
	{	
		if(value == s_SamplingRateTable[index][3])
			break;
	}
    
	if(SAMPLING_RATE_NUM == index)
	{
		index = 2; // If not find, set 48K.
	}

	g_UsbCtrlData[0] = s_SamplingRateTable[index][2];
	g_UsbCtrlData[1] = s_SamplingRateTable[index][1];
	g_UsbCtrlData[2] = s_SamplingRateTable[index][0];
	g_UsbCtrlData[3] = 0;
}

BYTE FreqToControlByte()
{
	BYTE index;

	for(index = 0; index<SAMPLING_RATE_NUM; ++index)
	{
		if((g_UsbCtrlData[0]==s_SamplingRateTable[index][2]) && 
				(g_UsbCtrlData[1]==s_SamplingRateTable[index][1]) &&
		   			(g_UsbCtrlData[2]==s_SamplingRateTable[index][0]))
			break;
	}

	if(SAMPLING_RATE_NUM == index)
	{
		index = 2; // If not find, set 48K.
	}

	return s_SamplingRateTable[index][3];
}

BYTE GetDmaFreq(BYTE endpoint)
{
	switch(endpoint)
	{
		case EP_MULTI_CH_PLAY:
#if (defined(_SUPPORT_384K_) || defined(_SUPPORT_768K_) || defined(_SUPPORT_1536K_))
			if(PERI_ReadByte(OSC_CTRL) == 0x02)
			{
				if((PERI_ReadByte(DMA_PLAY_8CH_L) & FREQ_MASK) >= DMA_192000)
				{
					if(s_Run1536K)
						return DMA_1536000;
					else if(s_Run768K)
						return DMA_768000;

					return DMA_384000;
				}
				else
				{
					if(s_Run1536K)
						return DMA_1411200;
					else if(s_Run768K)
						return DMA_705600;

					return DMA_352800;
				}
			}
#endif
			return PERI_ReadByte(DMA_PLAY_8CH_L) & FREQ_MASK;

		case EP_SPDIF_PLAY:
			return PERI_ReadByte(DMA_PLAY_SPDIF) & FREQ_MASK;

		case EP_SPDIF_REC:
			return PERI_ReadByte(DMA_REC_SPDIF) & FREQ_MASK;

		default:
			return DMA_48000;
	}
}

void SetDmaFreq(BYTE endpoint, BYTE freq)
{
	switch(endpoint)
	{
		case EP_MULTI_CH_PLAY:         

#if (defined(_SUPPORT_384K_) || defined(_SUPPORT_768K_) || defined(_SUPPORT_1536K_))
			if((freq == DMA_384000) || (freq == DMA_352800) || (freq == DMA_768000) || (freq == DMA_705600) || (freq == DMA_1536000) || (freq == DMA_1411200))
			{
				PERI_WriteByte(OSC_CTRL, 0x02); // OSC is 24.576/22.5792 MHz
			}
			else
			{
				PERI_WriteByte(OSC_CTRL, 0x03); // OSC is 49.152/45.1584 MHz
			}

			if((freq == DMA_768000) || (freq == DMA_705600))
				s_Run768K = TRUE;
			else
				s_Run768K = FALSE;

			if((freq == DMA_1536000) || (freq == DMA_1411200))
				s_Run1536K = TRUE;
			else
				s_Run1536K = FALSE;
#endif

			PERI_WriteByte(DMA_PLAY_8CH_L, (PERI_ReadByte(DMA_PLAY_8CH_L)&(~FREQ_MASK))|freq);
			break;

		case EP_SPDIF_PLAY:
			PERI_WriteByte(DMA_PLAY_SPDIF, (PERI_ReadByte(DMA_PLAY_SPDIF)&(~FREQ_MASK))|freq);
			break;

		case EP_SPDIF_REC:
			PERI_WriteByte(DMA_REC_SPDIF, (PERI_ReadByte(DMA_REC_SPDIF)&(~FREQ_MASK))|freq);

		default: break;
	}
}

static BYTE GetSamplingRate(BYTE freq)
{
	for(g_Index = 0; g_Index<SAMPLING_RATE_NUM; ++g_Index)
	{
		if(freq == s_SamplingRateTable[g_Index][3])
			break;
	}

	return s_SamplingRateTable[g_Index][4];
}

static void PlaybackResetRef()
{
	s_bPlaybackStartCount = 0;
	PERI_WriteByte(PAD_GP10_GP11_CTRL, PERI_ReadByte(PAD_GP10_GP11_CTRL) & ~bmBIT7); // enable playback
}	

static void PlaybackAddRef(BYTE index)
{
	s_bPlaybackStartCount |= (bmBIT0 << index);
}	

static void PlaybackReleaseRef(BYTE index)
{
	s_bPlaybackStartCount &= ~(bmBIT0 << index);
	
	if(s_bPlaybackStartCount == 0)
	{
		PERI_WriteByte(PAD_GP10_GP11_CTRL, PERI_ReadByte(PAD_GP10_GP11_CTRL) & ~bmBIT7); // enable playback
	}
}

static BYTE GetSpdifInSamplingRate()
{
	switch(PERI_ReadByte(SPDIF_IN_STATUS_3) & 0x0F)
	{
		case SPDIF_STATUS_44100:
			return DMA_44100;

		case SPDIF_STATUS_32000:
			return DMA_32000;

		case SPDIF_STATUS_88200:
			return DMA_88200;

		case SPDIF_STATUS_96000:
			return DMA_96000;

		case SPDIF_STATUS_64000:
			return DMA_64000;

		case SPDIF_STATUS_176400:
			return DMA_176400;

		case SPDIF_STATUS_192000:
			return DMA_192000;

		case SPDIF_STATUS_48000:
		default:
			return DMA_48000;
	}	
}

static void HandleSpdifIn()
{
	static BYTE code s_ClockRateChangeNotify[6] = {0, 1, 0, 1, 0, 23};
	static BYTE code s_ClockValidChangeNotify[6] = {0, 1, 0, 2, 0, 23};

	g_TempByte1 = PERI_ReadByte(SPDIF_CTRL_1);

	if(g_TempByte1 & bmBIT3)	// Sense
	{
		if(g_TempByte1 & bmBIT4)	// Lock
		{
			if(!g_SpdifLockStatus)
			{
				g_SpdifLockStatus = TRUE;
				SendInt15Data(s_ClockValidChangeNotify, 6);
			}
			else if(g_SpdifRateStatus != GetSpdifInSamplingRate())
			{
				g_SpdifRateStatus = GetSpdifInSamplingRate();
				SendInt15Data(s_ClockRateChangeNotify, 6);
			}
		}
		else		// No Lock
		{
			if(g_SpdifLockStatus)
			{
				g_SpdifLockStatus = FALSE;
				SendInt15Data(s_ClockValidChangeNotify, 6);
			}

			if((PERI_ReadByte(SPDIF_CTRL_2) & bmBIT0) == bmBIT0)
				PERI_WriteByte(SPDIF_CTRL_2, 0);
			else
				PERI_WriteByte(SPDIF_CTRL_2, bmBIT0);							
		}
	}
	else
	{
		if(g_SpdifLockStatus)
		{
			g_SpdifLockStatus = FALSE;
			SendInt15Data(s_ClockValidChangeNotify, 6);
		}
	}

	if(g_SpdifLockStatus && (GetDmaFreq(EP_SPDIF_REC)==g_SpdifRateStatus))
	{
		PERI_WriteByte(RECORD_ROUTING_L, PERI_ReadByte(RECORD_ROUTING_L) & ~bmBIT3);
	}
	else
	{
		PERI_WriteByte(RECORD_ROUTING_L, PERI_ReadByte(RECORD_ROUTING_L) | bmBIT3);
	}
}

BOOL GetCurrentMute(BYTE index)
{
	return (s_CurrentMute >> index) & bmBIT0;
}

void SetCurrentMute(BYTE index, BOOL mute)
{
	if(mute)
		s_CurrentMute = s_CurrentMute | (bmBIT0 << index);
	else
		s_CurrentMute = s_CurrentMute & (~(bmBIT0 << index));

	switch(g_MuteTable[index])
	{
		case 13: // Speaker
			if(mute)
				PERI_WriteByte(DMA_FIFO_MUTE, PERI_ReadByte(DMA_FIFO_MUTE) | bmBIT0);
			else
				PERI_WriteByte(DMA_FIFO_MUTE, PERI_ReadByte(DMA_FIFO_MUTE) & ~bmBIT0);
			break;

		case 14:	// SPDIF Out
			if(mute)
				PERI_WriteByte(DMA_FIFO_MUTE, PERI_ReadByte(DMA_FIFO_MUTE) | bmBIT1);
			else
				PERI_WriteByte(DMA_FIFO_MUTE, PERI_ReadByte(DMA_FIFO_MUTE) & ~bmBIT1);
			break;

		case 16:	// Mic In
			if(mute)
			{
				PERI_WriteByte(RECORD_MUTE, PERI_ReadByte(RECORD_MUTE) | bmBIT4);
			}
			else if(!s_RecordMute)
			{
				PERI_WriteByte(RECORD_MUTE, PERI_ReadByte(RECORD_MUTE) & (~bmBIT4));
			}
			break;

		case 17:	// Line In
			if(mute)
			{
				PERI_WriteByte(RECORD_MUTE, PERI_ReadByte(RECORD_MUTE) | bmBIT0);
			}
			else if(!s_RecordMute)
			{
				PERI_WriteByte(RECORD_MUTE, PERI_ReadByte(RECORD_MUTE) & (~bmBIT0));
			}
			break;

	}
}

WORD GetCurrentVolume(BYTE index)
{
	return s_CurrentVolume[index];
}

void SetCurrentVolume(BYTE index, WORD volume)
{
	s_CurrentVolume[index] = volume;
}

void SetRecordMute(BOOL mute)
{
	if(mute)
	{
		PERI_WriteByte(RECORD_MUTE, PERI_ReadByte(RECORD_MUTE) | bmBIT4 | bmBIT0);
		PERI_WriteByte(GPIO_DATA_H, PERI_ReadByte(GPIO_DATA_H) | bmBIT7);		
	}
	else
	{
		if(!GetCurrentMute(3))
			PERI_WriteByte(RECORD_MUTE, PERI_ReadByte(RECORD_MUTE) & (~bmBIT4));

		if(!GetCurrentMute(4))
			PERI_WriteByte(RECORD_MUTE, PERI_ReadByte(RECORD_MUTE) & (~bmBIT0));

		PERI_WriteByte(GPIO_DATA_H, PERI_ReadByte(GPIO_DATA_H) & (~bmBIT7));
	}		

	s_RecordMute = mute;
}

void AudioInit()
{
	// For improving jitter
	PERI_WriteByte(PAD_GP24_GP25_CTRL, 0xB8);	// Flash driving current 2mA

	// Mute unused I2S MCLK
	//PERI_WriteByte(I2S_PLAY_SPDIF_H, bmBIT4);
	PERI_WriteByte(I2S_PLAY_2CH_H, bmBIT4);
	PERI_WriteByte(I2S_REC_8CH_H, bmBIT4);
	//PERI_WriteByte(I2S_REC_2CH_H, bmBIT4);
	//PERI_WriteByte(I2S_REC_SPDIF_H, bmBIT4);

	PERI_WriteByte(DMA_FIFO_FLUSH, bmBIT6);	// Flush FIFO automatically when FIFO is full

	PERI_WriteByte(PLAYBACK_ROUTING_L, MULTI_CH_PLAY_ROUTE_I2S|SPDIF_PLAY_ROUTE_SPDIF); 
	PERI_WriteByte(RECORD_ROUTING_L, SPDIF_REC_ROUTE_SPDIF);

	s_bPlaybackStartCount = 0;

#ifdef _EXT_OSC_45_49_
	PERI_WriteByte(CLOCK_ROUTE, 0xCD); // Enable XD6, XD7 external ocsilator, discard packet if CRC error
	PERI_WriteByte(OSC_CTRL, 0x03); // OSC is 49.152/45.1584 MHz
	PERI_WriteByte(PLL_CTRL, 0x26);	// Turn off PLL-A, PLL-B, PLL-3. Turn on PLL-1, PLL-2
#elif defined(_EXT_OSC_22_24_)
	PERI_WriteByte(CLOCK_ROUTE, 0xCD); // Enable XD6, XD7 external ocsilator, discard packet if CRC error
	PERI_WriteByte(OSC_CTRL, 0x02); // OSC is 24.576/22.5792 MHz
	PERI_WriteByte(PLL_CTRL, 0x26);	// Turn off PLL-A, PLL-B, PLL-3. Turn on PLL-1, PLL-2
#else
	PERI_WriteByte(CLOCK_ROUTE, 0xC1); // Discard packet if CRC error
#endif

#ifdef _FEEDBACK_
	if(g_IsAudioClass20)
		PERI_WriteByte(FEEDBACK_CTRL_L, 0x00);
	else
		PERI_WriteByte(FEEDBACK_CTRL_L, 0xC0);

	PERI_WriteWord(FEEDBACK_CTRL2_L, 0x0249);	// Feedback 1/32 sample, period 32ms

	PERI_WriteByte(PLL_ADJUST_PERIOD, 0x00);

#ifdef _MCU_FEEDBACK_
	s_MultiChFeedbackStart = FALSE;
	s_SpdifFeedbackStart = FALSE;

	PERI_WriteByte(MISC_FUNCTION_CTRL, PERI_ReadByte(MISC_FUNCTION_CTRL) | bmBIT6); //set playback sync. to SOF
	USB_HANDSHAKE3 = 0x1D;	// Enable 32ms SOF interrupt and MCU feedback control
#endif

#else
	if(g_IsAudioClass20)
		PERI_WriteByte(FEEDBACK_CTRL_L, 0x3F);
	else
		PERI_WriteByte(FEEDBACK_CTRL_L, 0xBF);

	PERI_WriteByte(PLL_ADJUST_PERIOD, 32);	// PLL adjust period 32ms
	PERI_WriteByte(MISC_FUNCTION_CTRL, PERI_ReadByte(MISC_FUNCTION_CTRL) | bmBIT6); // Play sync. to SOF
	PERI_WriteByte(PLL_THRES_LIMIT_H, 0x07);	// Disable threshold limitation
#endif

	PlaybackResetRef();

	for(g_Index=0; g_Index<MUTE_DSCR_NUM; ++g_Index)
		SetCurrentMute(g_Index, (s_CurrentMute>>g_Index)&bmBIT0);

	for(g_Index=0; g_Index<VOLUME_DSCR_NUM; ++g_Index)
		SetCurrentVolume(g_Index, s_CurrentVolume[g_Index]);

#ifdef _DSD_
	PERI_WriteByte(GPIO_DATA_H, PERI_ReadByte(GPIO_DATA_H) & ~bmBIT7);  // GPIO15 -> DSD off
	s_DsdState = DSD_OFF;
#endif
}

#ifdef _MCU_FEEDBACK_

void HandleIsoFeedback()
{
	if(USB_HANDSHAKE1 & bmBIT4)
	{
		USB_HANDSHAKE2 = bmBIT4;	// Clear interrupt status

		s_MultiChCount = PERI_ReadWord(MULTI_CH_FIFO_REMAIN_L);
		s_SpdifCount = PERI_ReadWord(SPDIF_FIFO_REMAIN_L);

		if((!s_MultiChFeedbackStart) && IS_PLAYING(PLAY_8CH))
		{
			g_TempWord1 = (MULTI_CH_PLAYBACK_COUNT[1] << 8) | MULTI_CH_PLAYBACK_COUNT[0];

			if(s_MultiChCountOld != g_TempWord1)
			{
				s_MultiChFeedbackStart = TRUE;
				s_MultiChCountOld = s_MultiChCount;

				if((!g_IsAudioClass20) && g_UsbIsHighSpeed)
				{
					s_MultiChThreshold = s_MultiChCount + 64;
				}
			}
		}

		if((!s_SpdifFeedbackStart) && IS_PLAYING(PLAY_SPDIF))
		{
			g_TempWord1 = (SPDIF_PLAYBACK_COUNT[1] << 8) | SPDIF_PLAYBACK_COUNT[0];

			if(s_SpdifCountOld != g_TempWord1)
			{
				s_SpdifFeedbackStart = TRUE;
				s_SpdifCountOld = s_SpdifCount;

				if((!g_IsAudioClass20) && g_UsbIsHighSpeed)
				{
					s_SpdifThreshold = s_SpdifCount + 64;
				}
			}
		}

		if(s_MultiChFeedbackStart)
		{
			//g_InputReport.reserved1 = MSB(s_MultiChCount);
			//g_InputReport.reserved2 = LSB(s_MultiChCount);
			//g_InputReport.offset_high = MSB(s_MultiChThreshold);
			//g_InputReport.offset_low = LSB(s_MultiChThreshold);
#if (defined(_SUPPORT_384K_) || defined(_SUPPORT_768K_) || defined(_SUPPORT_1536K_))
			if(PERI_ReadByte(OSC_CTRL) == 0x02)
			{
				if((PERI_ReadByte(DMA_PLAY_8CH_L) & FREQ_MASK) == DMA_192000)
					g_Index = 11;	// 384000
				else
					g_Index = 10;	// 352800

				if(s_Run1536K)
					g_Index += 4;
				else if(s_Run768K)
					g_Index += 2;
			}
			else
			{
				g_Index = (GetDmaFreq(EP_MULTI_CH_PLAY) & FREQ_MASK) >> 3;
			}
#else
			g_Index = (GetDmaFreq(EP_MULTI_CH_PLAY) & FREQ_MASK) >> 3;
#endif

			if(s_MultiChCount > s_MultiChThreshold)
			{
				g_TempWord1 = s_MultiChCount - s_MultiChThreshold;

				if(s_MultiChCount < s_MultiChCountOld)
				{
					//g_TempWord1 >>= 5;

					g_TempByte2 = s_MultiChCountOld - s_MultiChCount;

					if(g_TempWord1 > g_TempByte2)
						g_TempWord1 -= g_TempByte2;
					else
						g_TempWord1 = 0;
				}

				s_MultiChCountOld = s_MultiChCount;

				if(s_MultiChFeedbackRatio)
				{
					g_TempWord1 /= s_MultiChFeedbackRatio;
				}

				if(g_TempWord1)
				{
					if(s_Run1536K)
					{
						if(g_TempWord1 > 8)
							g_TempWord1 = 8;
					}
					else if(s_Run768K)
					{
						if(g_TempWord1 > 4)
							g_TempWord1 = 4;
					}
					else
					{
						if(g_TempWord1 > 2)
							g_TempWord1 = 2;
					}

					//g_InputReport.source = LSB(g_TempWord1) | bmBIT7;

					g_TempWord1 = s_FeedbackTable[g_Index].mid - g_TempWord1;
				}
			}
			else
			{
				g_TempWord1 = s_MultiChThreshold - s_MultiChCount;

				if(s_MultiChCount > s_MultiChCountOld)
				{
					//g_TempWord1 >>= 5;

					g_TempByte2 = s_MultiChCount - s_MultiChCountOld;

					if(g_TempWord1 > g_TempByte2)
						g_TempWord1 -= g_TempByte2;
					else
						g_TempWord1 = 0;
				}

				s_MultiChCountOld = s_MultiChCount;

				if(s_MultiChFeedbackRatio)
				{
					g_TempWord1 /= s_MultiChFeedbackRatio;
				}

				if(g_TempWord1)
				{
					if(s_Run1536K)
					{
						if(g_TempWord1 > 8)
							g_TempWord1 = 8;
					}
					else if(s_Run768K)
					{
						if(g_TempWord1 > 4)
							g_TempWord1 = 4;
					}
					else
					{
						if(g_TempWord1 > 2)
							g_TempWord1 = 2;
					}

					//g_InputReport.source = LSB(g_TempWord1);

					g_TempWord1 = s_FeedbackTable[g_Index].mid + g_TempWord1;
				}
			}

			if(g_TempWord1)
			{
				if(g_IsAudioClass20 && g_UsbIsHighSpeed)
				{
					MULTI_CH_FEEDBACK_DATA[0] = s_FeedbackTable[g_Index].end;
					MULTI_CH_FEEDBACK_DATA[1] = LSB(g_TempWord1);
					MULTI_CH_FEEDBACK_DATA[2] = MSB(g_TempWord1);
				}
				else
				{
					MULTI_CH_FEEDBACK_DATA[0] = (s_FeedbackTable[g_Index].end << 3);
					g_TempWord1 = ((g_TempWord1<<3) | ((s_FeedbackTable[g_Index].end & 0xE0) >> 5));
					MULTI_CH_FEEDBACK_DATA[1] = LSB(g_TempWord1);
					MULTI_CH_FEEDBACK_DATA[2] = MSB(g_TempWord1);
				}

				MULTI_CH_FEEDBACK_DATA[3] = 0;
				USB_HANDSHAKE4 = bmBIT0;
			}
			else
			{
#if (defined(_SUPPORT_384K_) || defined(_SUPPORT_768K_) || defined(_SUPPORT_1536K_))
				if(g_Index >= 10)
				{
					MULTI_CH_FEEDBACK_DATA[0] = s_FeedbackTable[g_Index].end;
					MULTI_CH_FEEDBACK_DATA[1] = LSB(s_FeedbackTable[g_Index].mid);
					MULTI_CH_FEEDBACK_DATA[2] = MSB(s_FeedbackTable[g_Index].mid);
					MULTI_CH_FEEDBACK_DATA[3] = 0;
					USB_HANDSHAKE4 = bmBIT0;
				}
				else
				{
					USB_HANDSHAKE4 = bmBIT3;	// Reset feedback data
				}
#else
				USB_HANDSHAKE4 = bmBIT3;	// Reset feedback data
#endif
			}

			//SendInt4Data((BYTE*)(&g_InputReport), INPUT_REPORT_SIZE);
		}

		if(s_SpdifFeedbackStart)
		{
			//g_InputReport.reserved1 = MSB(s_SpdifCount);
			//g_InputReport.reserved2 = LSB(s_SpdifCount);
			//g_InputReport.offset_high = MSB(s_SpdifThreshold);
			//g_InputReport.offset_low = LSB(s_SpdifThreshold);

			g_Index = PERI_ReadByte(DMA_PLAY_SPDIF) >> 3;

			if(s_SpdifCount > s_SpdifThreshold)
			{
				g_TempWord1 = s_SpdifCount - s_SpdifThreshold;

				if(s_SpdifCount < s_SpdifCountOld)
					g_TempWord1 >>= 5;

				s_SpdifCountOld = s_SpdifCount;

				if(s_SpdifFeedbackRatio)
				{
					g_TempWord1 /= s_SpdifFeedbackRatio;
				}

				if(g_TempWord1)
				{
					if(g_TempWord1 > 2)
						g_TempWord1 = 2;

					//g_InputReport.source = LSB(g_TempWord1) | bmBIT7;

					g_TempWord1 = s_FeedbackTable[g_Index].mid - g_TempWord1;
				}
			}
			else
			{
				g_TempWord1 = s_SpdifThreshold - s_SpdifCount;

				if(s_SpdifCount > s_SpdifCountOld)
					g_TempWord1 >>= 5;

				s_SpdifCountOld = s_SpdifCount;

				if(s_SpdifFeedbackRatio)
				{
					g_TempWord1 /= s_SpdifFeedbackRatio;
				}

				if(g_TempWord1)
				{
					if(g_TempWord1 > 2)
						g_TempWord1 = 2;

					//g_InputReport.source = LSB(g_TempWord1);

					g_TempWord1 = s_FeedbackTable[g_Index].mid + g_TempWord1;
				}
			}

			if(g_TempWord1)
			{
				if(g_IsAudioClass20 && g_UsbIsHighSpeed)
				{
					SPDIF_FEEDBACK_DATA[0] = s_FeedbackTable[g_Index].end;
					SPDIF_FEEDBACK_DATA[1] = LSB(g_TempWord1);
					SPDIF_FEEDBACK_DATA[2] = MSB(g_TempWord1);
				}
				else
				{
					SPDIF_FEEDBACK_DATA[0] = (s_FeedbackTable[g_Index].end << 3);
					g_TempWord1 = ((g_TempWord1<<3) | ((s_FeedbackTable[g_Index].end & 0xE0) >> 5));
					SPDIF_FEEDBACK_DATA[1] = LSB(g_TempWord1);
					SPDIF_FEEDBACK_DATA[2] = MSB(g_TempWord1);
				}

				SPDIF_FEEDBACK_DATA[3] = 0;
				USB_HANDSHAKE4 = bmBIT1;
			}
			else
			{
				USB_HANDSHAKE4 = bmBIT4;	// Reset feedback data
			}

			//SendInt4Data((BYTE*)(&g_InputReport), INPUT_REPORT_SIZE);
		}
	}
}

#endif

void AudioProcess()
{
#ifdef _MCU_FEEDBACK_
	HandleIsoFeedback();
#endif

#ifdef _DSD_

	if(s_DsdState == DSD_ON)
	{
		g_TempWord1 = (MULTI_CH_PLAYBACK_COUNT[1] << 8) | MULTI_CH_PLAYBACK_COUNT[0];
		if(s_MultiChCountOld != g_TempWord1)
		{
			s_DsdState = DSD_RUNNING;
			PERI_WriteByte(GPIO_DATA_H, PERI_ReadByte(GPIO_DATA_H) | bmBIT7);  // GPIO15 -> DSD on
		}
	}

	if((s_DsdState==DSD_RUNNING) && (PERI_ReadWord(MULTI_CH_FIFO_REMAIN_L) < 100))
	{
		s_DsdState = DSD_STOPPING;
		PERI_WriteByte(GPIO_DATA_H, PERI_ReadByte(GPIO_DATA_H) & ~bmBIT7);  // GPIO15 -> DSD off
	}

	if((s_DsdState==DSD_STOPPING) && (PERI_ReadWord(MULTI_CH_FIFO_REMAIN_L) >= 176))
	{
		s_DsdState = DSD_RUNNING;
		PERI_WriteByte(GPIO_DATA_H, PERI_ReadByte(GPIO_DATA_H) | bmBIT7);  // GPIO15 -> DSD on
	}

#endif

	if(g_Tick10ms > 10)
	{
		g_Tick10ms = 0;

		HandleSpdifIn();

		if(s_bPlaybackStartCount)
		{
			g_TempByte1 = PERI_ReadByte(PAD_GP10_GP11_CTRL);

			if(!(g_TempByte1 & bmBIT7))
			{
				PERI_WriteByte(PAD_GP10_GP11_CTRL, PERI_ReadByte(PAD_GP10_GP11_CTRL) | bmBIT7); // disable playback
			}
			else
			{
				PERI_WriteByte(PAD_GP10_GP11_CTRL, PERI_ReadByte(PAD_GP10_GP11_CTRL) & ~bmBIT7); // enable playback
			}
		}
	}
}

BOOL PlayMultiChStart(BYTE ch, BYTE format)
{
	g_TempByte1 = GetDmaFreq(EP_MULTI_CH_PLAY);

	PERI_WriteByte(DMA_PLAY_8CH_L, (PERI_ReadByte(DMA_PLAY_8CH_L)&(~RESOLUTION_MASK))|format);

	g_Index = (g_TempByte1 & FREQ_MASK) >> 3;

	// Set sampling rate
	switch(g_TempByte1)
	{
		case DMA_44100:
			g_TempWord1 = BCLK_LRCK_64|MCLK_LRCK_512|I2S_44100;
			g_TempByte2 = bmBIT0; // 0 (F3), 0 (F2), 0(F1), 1(F0) -> 44.1kHz
			break;

		case DMA_48000:
			g_TempWord1 = BCLK_LRCK_64|MCLK_LRCK_512|I2S_48000;
			g_TempByte2 = bmBIT1; // 0 (F3), 0 (F2), 1(F1), 0(F0) -> 48kHz
			break;

		case DMA_88200:
			g_TempWord1 = BCLK_LRCK_64|MCLK_LRCK_256|I2S_88200;
			g_TempByte2 = bmBIT1 | bmBIT0; // 0 (F3), 0 (F2), 1(F1), 1(F0) -> 88.2kHz
			break;

		case DMA_96000:
			g_TempWord1 = BCLK_LRCK_64|MCLK_LRCK_256|I2S_96000;
			g_TempByte2 = bmBIT2; // 0 (F3), 1 (F2), 0(F1), 0(F0) -> 96kHz
			break;

		case DMA_176400:
			g_TempWord1 = BCLK_LRCK_64|MCLK_LRCK_128|I2S_176400;
			g_TempByte2 = bmBIT2 | bmBIT0; // 0 (F3), 1 (F2), 0(F1), 1(F0) -> 176.4kHz
			break;

		case DMA_192000:
			g_TempWord1 = BCLK_LRCK_64|MCLK_LRCK_128|I2S_192000;
			g_TempByte2 = bmBIT2 | bmBIT1; // 0 (F3), 1 (F2), 1(F1), 0(F0) -> 192kHz
			break;

		case DMA_352800:
			g_TempByte2 = bmBIT2 | bmBIT1 | bmBIT0; // 0 (F3), 1 (F2), 1(F1), 1(F0) -> 352.8kHz

			g_TempWord1 = BCLK_LRCK_64|MCLK_LRCK_128|I2S_176400;
			break;

		case DMA_384000:
			g_TempByte2 = bmBIT3; // 1 (F3), 0 (F2), 0(F1), 0(F0) -> 384kHz

			g_TempWord1 = BCLK_LRCK_64|MCLK_LRCK_128|I2S_192000;
			break;

		case DMA_705600:
			g_TempWord1 = BCLK_LRCK_64|MCLK_LRCK_128|I2S_176400;
			g_TempByte2 = bmBIT3 | bmBIT0; // 1 (F3), 0 (F2), 0(F1), 1(F0) -> 705.6kHz
			break;

		case DMA_768000:
			g_TempWord1 = BCLK_LRCK_64|MCLK_LRCK_128|I2S_192000;
			g_TempByte2 = bmBIT3 | bmBIT1; // 1 (F3), 0 (F2), 1(F1), 0(F0) -> 768kHz
			break;

		case DMA_1411200:
			g_TempWord1 = BCLK_LRCK_64|MCLK_LRCK_128|I2S_176400;
			g_TempByte2 = bmBIT3 | bmBIT2 | bmBIT0; // 1 (F3), 1 (F2), 0(F1), 1(F0) -> 1411.2kHz
			break;

		case DMA_1536000:
			g_TempWord1 = BCLK_LRCK_64|MCLK_LRCK_128|I2S_192000;
			g_TempByte2 = bmBIT3 | bmBIT2 | bmBIT1; // 1 (F3), 1 (F2), 1(F1), 0(F0) -> 1536kHz
			break;

		default:
			return FALSE;
	}

#ifdef _DSD_
	if(ch == DMA_4CH) // DSD data
		g_TempWord1 |= I2S_MASTER | LEFT_JUST;
	else
		g_TempWord1 |= I2S_MASTER | I2S_MODE;
#else
	g_TempWord1 |= I2S_MASTER | I2S_MODE;
#endif

	// Set bit resolution
	switch(format & RESOLUTION_MASK)
	{
		case DMA_16Bit:
#ifdef _MCU_FEEDBACK_
			s_MultiChFeedbackRatio = 1;
#endif

			g_TempWord1 |= I2S_16Bit;
			break;

		case DMA_24Bit:
		case DMA_24Bit_32Container:
#ifdef _MCU_FEEDBACK_
			s_MultiChFeedbackRatio = 2;
#endif

			g_Index += 10;
			g_TempWord1 |= I2S_24Bit;
			break;

		case DMA_32Bit:
#ifdef _MCU_FEEDBACK_
			s_MultiChFeedbackRatio = 2;
#endif

			g_Index += 10;
			g_TempWord1 |= I2S_32Bit;
	}

	SetPinFreq(g_TempByte2);

	PERI_WriteByte(I2S_PLAY_8CH_H, PERI_ReadByte(I2S_PLAY_8CH_H) | bmBIT4); // Mute MCLK
	PERI_WriteWord(I2S_PLAY_8CH_L, g_TempWord1|MCLK_MUTE);	// Set I2S format
	PERI_WriteByte(I2S_PLAY_8CH_H, PERI_ReadByte(I2S_PLAY_8CH_H) & ~bmBIT4); // Unmute MCLK

	PERI_WriteByte(DMA_PLAY_8CH_H, ch);	// Set channel numbers
	switch(ch)
	{
		case DMA_2CH:
			PERI_WriteByte(PLAYBACK_ROUTING_H, PLAYBACK_ROUTING_2CH);
			break;

#ifdef _DSD_
		case DMA_4CH: // DSD
			s_DsdState = DSD_ON;

#ifdef _MCU_FEEDBACK_
			s_MultiChFeedbackRatio <<= 1;
#endif

			g_Index += 20;	
			PERI_WriteByte(PLAYBACK_ROUTING_H, PLAYBACK_ROUTING_2CH);
			break;
#else
		case DMA_4CH:
#ifdef _MCU_FEEDBACK_
			s_MultiChFeedbackRatio <<= 1;
#endif

			g_Index += 20;
			PERI_WriteByte(PLAYBACK_ROUTING_H, PLAYBACK_ROUTING_4CH);
			break;
#endif
	}

	if(g_IsAudioClass20 && g_UsbIsHighSpeed)
		PERI_WriteByte(STARTING_THRESHOLD_MUTICH_L, s_uFrameThresholdTable[g_Index]);
	else
		PERI_WriteByte(STARTING_THRESHOLD_MUTICH_L, s_mSecondThresholdTable[g_Index]);

#ifdef _MCU_FEEDBACK_
	if(g_IsAudioClass20 && g_UsbIsHighSpeed)
		s_MultiChThreshold = s_uFrameThresholdTable[g_Index];
	else
		s_MultiChThreshold = s_mSecondThresholdTable[g_Index];

	USB_HANDSHAKE4 = bmBIT3;	// Reset feedback data
	s_MultiChCountOld = (MULTI_CH_PLAYBACK_COUNT[1] << 8) | MULTI_CH_PLAYBACK_COUNT[0];

#if (defined(_SUPPORT_384K_) || defined(_SUPPORT_768K_) || defined(_SUPPORT_1536K_))
	if((g_TempByte1==DMA_384000) || (g_TempByte1==DMA_352800) || (g_TempByte1==DMA_768000) || (g_TempByte1==DMA_705600) || (g_TempByte1==DMA_1536000) || (g_TempByte1==DMA_1411200))
	{
		if(g_TempByte1 == DMA_1536000)
			g_Index = 15;
		else if(g_TempByte1 == DMA_1411200)
			g_Index = 14;
		else if(g_TempByte1 == DMA_768000)
			g_Index = 13;
		else if(g_TempByte1 == DMA_705600)
			g_Index = 12;
		else if(g_TempByte1 == DMA_384000)
			g_Index = 11;
		else
			g_Index = 10;

		MULTI_CH_FEEDBACK_DATA[0] = s_FeedbackTable[g_Index].end;
		MULTI_CH_FEEDBACK_DATA[1] = LSB(s_FeedbackTable[g_Index].mid);
		MULTI_CH_FEEDBACK_DATA[2] = MSB(s_FeedbackTable[g_Index].mid);
		MULTI_CH_FEEDBACK_DATA[3] = 0;
		USB_HANDSHAKE4 = bmBIT0;
	}
#endif
#endif

	PERI_WriteByte(DMA_PLAY_8CH_L, PERI_ReadByte(DMA_PLAY_8CH_L)|DMA_START);
	PlaybackAddRef(PLAY_8CH);

	return TRUE;
}

BOOL PlaySpdifStart(BYTE format)
{
	g_TempByte1 = GetDmaFreq(EP_SPDIF_PLAY);
	PERI_WriteByte(DMA_PLAY_SPDIF, (format&0x7F)|g_TempByte1);

	g_TempByte2 = PERI_ReadByte(SPDIF_CTRL_3) & 0xF8;

	g_Index = (g_TempByte1 & FREQ_MASK) >> 3;

	// Set sampling rate
	switch(g_TempByte1)
	{
		case DMA_32000:
			g_TempByte2 |= SPDIF_CTRL_32000;
			break;

		case DMA_44100:
			g_TempByte2 |= SPDIF_CTRL_44100;
			break;

		case DMA_48000:
			g_TempByte2 |= SPDIF_CTRL_48000;
			break;

		case DMA_88200:
			g_TempByte2 |= SPDIF_CTRL_88200;
			break;		

		case DMA_96000:
			g_TempByte2 |= SPDIF_CTRL_96000;
			break;		

		case DMA_176400:
			g_TempByte2 |= SPDIF_CTRL_176400;
			break;

		case DMA_192000:
			g_TempByte2 |= SPDIF_CTRL_192000;
			break;

		default:
			return FALSE;
	}

	PERI_WriteByte(SPDIF_CTRL_3, g_TempByte2);

	// Set bit resolution
	switch(format & RESOLUTION_MASK)
	{
		case DMA_16Bit:
#ifdef _MCU_FEEDBACK_
			s_SpdifFeedbackRatio = 1;
#endif

			break;

		case DMA_24Bit:
		case DMA_24Bit_32Container:
		case DMA_32Bit:
#ifdef _MCU_FEEDBACK_
			s_SpdifFeedbackRatio = 2;
#endif

			g_Index += 10;
			break;
	}

	if(format & NON_PCM)
	{
		PERI_WriteByte(SPDIF_OUT0_STATUS_L, (PERI_ReadByte(SPDIF_OUT0_STATUS_L) | (bmBIT1|bmBIT5)) | bmBIT0);			
	}
	else
	{
		PERI_WriteByte(SPDIF_OUT0_STATUS_L, (PERI_ReadByte(SPDIF_OUT0_STATUS_L) & ~(bmBIT1|bmBIT5)) | bmBIT0);			
	}

	if(g_IsAudioClass20 && g_UsbIsHighSpeed)
		PERI_WriteByte(STARTING_THRESHOLD_SPDIF_L, s_uFrameThresholdTable[g_Index]);
	else
		PERI_WriteByte(STARTING_THRESHOLD_SPDIF_L, s_mSecondThresholdTable[g_Index]);

#ifdef _MCU_FEEDBACK_
	if(g_IsAudioClass20 && g_UsbIsHighSpeed)
		s_SpdifThreshold = s_uFrameThresholdTable[g_Index];
	else
		s_SpdifThreshold = s_mSecondThresholdTable[g_Index];

	USB_HANDSHAKE4 = bmBIT4;	// Reset feedback data
	s_SpdifCountOld = (SPDIF_PLAYBACK_COUNT[1] << 8) | SPDIF_PLAYBACK_COUNT[0];
#endif

	PERI_WriteByte(DMA_PLAY_SPDIF, PERI_ReadByte(DMA_PLAY_SPDIF)|DMA_START);

	PlaybackAddRef(PLAY_SPDIF);
	return TRUE;
}

BOOL RecSpdifStart(BYTE format)
{
	g_TempByte1 = GetDmaFreq(EP_SPDIF_REC);
	PERI_WriteByte(DMA_REC_SPDIF, format|g_TempByte1);

	g_TempWord2 = GetSamplingRate(g_TempByte1);

	if(g_SpdifLockStatus && (g_TempByte1==g_SpdifRateStatus))
	{
		PERI_WriteByte(RECORD_ROUTING_L, PERI_ReadByte(RECORD_ROUTING_L) & ~bmBIT3);
	}
	else
	{
		PERI_WriteByte(RECORD_ROUTING_L, PERI_ReadByte(RECORD_ROUTING_L) | bmBIT3);
	}

	if(g_IsAudioClass20 && g_UsbIsHighSpeed)
		g_TempWord2 = (g_TempWord2/8)+2;
	else
		g_TempWord2 = g_TempWord2+2;

	// Set sampling rate
	switch(g_TempByte1)
	{
		case DMA_32000:
			g_TempWord1 = BCLK_LRCK_128|I2S_MASTER|MCLK_LRCK_256|I2S_MODE|I2S_32000;
			break;

		case DMA_44100:
			g_TempWord1 = BCLK_LRCK_128|I2S_MASTER|MCLK_LRCK_256|I2S_MODE|I2S_44100;
			break;

		case DMA_48000:
			g_TempWord1 = BCLK_LRCK_128|I2S_MASTER|MCLK_LRCK_256|I2S_MODE|I2S_48000;
			break;

		case DMA_64000:
			g_TempWord1 = BCLK_LRCK_128|I2S_MASTER|MCLK_LRCK_256|I2S_MODE|I2S_64000;
			break;

		case DMA_88200:
			g_TempWord1 = BCLK_LRCK_64|I2S_MASTER|MCLK_LRCK_256|I2S_MODE|I2S_88200;
			break;

		case DMA_96000:
			g_TempWord1 = BCLK_LRCK_64|I2S_MASTER|MCLK_LRCK_256|I2S_MODE|I2S_96000;
			break;

		case DMA_176400:
			g_TempWord1 = BCLK_LRCK_64|I2S_MASTER|MCLK_LRCK_128|I2S_MODE|I2S_176400;
			break;

		case DMA_192000:
			g_TempWord1 = BCLK_LRCK_64|I2S_MASTER|MCLK_LRCK_128|I2S_MODE|I2S_192000;
			break;

		default:
			return FALSE;
	}

	// Set bit resolution
	switch(format & RESOLUTION_MASK)
	{
		case DMA_16Bit:
			g_TempWord1 |= I2S_16Bit;
			g_TempWord2 *= 4;
			break;

		case DMA_24Bit:
			g_TempWord1 |= I2S_24Bit;
			g_TempWord2 *= 6;
			break;

		case DMA_24Bit_32Container:
			g_TempWord1 |= I2S_24Bit;
			g_TempWord2 *= 8;
			break;

		case DMA_32Bit:
			g_TempWord1 |= I2S_32Bit;
			g_TempWord2 *= 8;
	}

	PERI_WriteByte(I2S_REC_SPDIF_H, PERI_ReadByte(I2S_REC_SPDIF_H) | bmBIT4); // Mute MCLK
	PERI_WriteWord(I2S_REC_SPDIF_L, g_TempWord1|MCLK_MUTE);	// Set I2S format
	PERI_WriteByte(I2S_REC_SPDIF_H, PERI_ReadByte(I2S_REC_SPDIF_H) & ~bmBIT4); // Unmute MCLK

	PERI_WriteWord(MAX_DATA_REC_SPDIF_L, g_TempWord2);

	PERI_WriteByte(SPDIF_IN_STATUS_0, 1);
	PERI_WriteByte(DMA_REC_SPDIF, PERI_ReadByte(DMA_REC_SPDIF)|DMA_START);
	return TRUE;
}

BOOL PlayMultiChStop()
{
	PERI_WriteByte(DMA_PLAY_8CH_L, PERI_ReadByte(DMA_PLAY_8CH_L) & ~bmBIT0);
	PlaybackReleaseRef(PLAY_8CH);

#ifdef _MCU_FEEDBACK_
	s_MultiChFeedbackStart = FALSE;
#endif

#ifdef _DSD_
	s_DsdState = DSD_OFF;

	PERI_WriteByte(GPIO_DATA_H, PERI_ReadByte(GPIO_DATA_H) & ~bmBIT7);  // GPIO15 -> DSD off
#endif

	return TRUE;
}

BOOL PlaySpdifStop()
{
	PERI_WriteByte(SPDIF_OUT0_STATUS_L, PERI_ReadByte(SPDIF_OUT0_STATUS_L) & ~bmBIT0);			

	PERI_WriteByte(DMA_PLAY_SPDIF, PERI_ReadByte(DMA_PLAY_SPDIF) & ~bmBIT0);
	PlaybackReleaseRef(PLAY_SPDIF);	

#ifdef _MCU_FEEDBACK_
	s_SpdifFeedbackStart = FALSE;
#endif

	return TRUE;
}

BOOL RecSpdifStop()
{
	PERI_WriteByte(SPDIF_IN_STATUS_0, 0);
	PERI_WriteByte(DMA_REC_SPDIF, PERI_ReadByte(DMA_REC_SPDIF) & ~bmBIT0);
	return TRUE;
}

void AudioStop()
{
	PlayMultiChStop();
	PlaySpdifStop();
	RecSpdifStop();

	PlaybackResetRef();
}
