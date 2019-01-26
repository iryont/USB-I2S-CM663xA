#ifndef _CONFIG_H_
#define _CONFIG_H_

//-----------------------------------------------------------------------------
// Configuration Switch
//-----------------------------------------------------------------------------
//#define _FULL_SPEED_ONLY_
//#define _AUDIO_CLASS_20_ONLY_
//#define _I2C_SLAVE_SUPPORT_

//-----------------------------------------------------------------------------
// IDs in Descriptor
//-----------------------------------------------------------------------------

#define _FEEDBACK_
#define _MCU_FEEDBACK_
//#define _EXT_OSC_22_24_
#define _EXT_OSC_45_49_
#define _SUPPORT_32BIT_

#ifdef _EXT_OSC_45_49_
#define _SUPPORT_384K_
#endif

//#define _SUPPORT_768K_
//#define _SUPPORT_1536K_
#define _DSD_

#define VENDOR_ID 0x8C0D
//#define PRODUCT_ID 0x2020
#define PRODUCT_ID 0x4101
#define VERSION_ID 0x3333

#endif

