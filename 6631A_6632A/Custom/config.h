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
//#define _FPGA_SLAVE_

#ifdef _EXT_OSC_45_49_
#define _SUPPORT_384K_
#endif

#ifdef _FPGA_SLAVE_
#define _SUPPORT_768K_
#define _SUPPORT_1536K_
#define _DSD_
#endif

#define VENDOR_ID 0x8C0D
//#define PRODUCT_ID 0x2020
#define PRODUCT_ID 0x4101

#ifdef _FPGA_SLAVE_
#define VERSION_ID 0x2222
#else
#define VERSION_ID 0x1111
#endif

#endif

