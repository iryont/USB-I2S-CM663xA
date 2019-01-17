#ifndef _CM66XX_H_
#define _CM66XX_H_

// The registers of CM66xx are defined here. We use cm66xx.h for register 
// address allocation by using "#define ALLOCATE_EXTERN". 
// When using "#define ALLOCATE_EXTERN", you get (for instance): 
// xdata volatile BYTE OUT7BUF[64]   _at_   0x7B40;
// Incidently, these lines will not generate any space in the resulting hex 
// file; they just bind the symbols to the addresses for compilation. 
// You just need to put "#define ALLOCATE_EXTERN" in your main program file; 
// i.e. fw.c or a stand-alone C source file. 
// Without "#define ALLOCATE_EXTERN", you just get the external reference: 
// extern xdata volatile BYTE OUT7BUF[64]   ;//   0x7B40;
// This uses the concatenation operator "##" to insert a comment "//" 
// to cut off the end of the line, "_at_   0x7B40;", which is not wanted.
#ifdef ALLOCATE_EXTERN
#define EXTERN
#define _AT_ _at_
#else
#define EXTERN extern
#define _AT_ ;/ ## /
#endif

#define STATUS_ACK   0x00
#define STATUS_NAK   0x01
#define STATUS_STALL 0x02 

// Peripherals Interface
EXTERN xdata volatile BYTE PERI_DEST_ADDR_L    _AT_ 0x0000;  // Destination address of peripherals low byte
EXTERN xdata volatile BYTE PERI_HANDSHAKE      _AT_ 0x0001;  // Peripheral handshake control
EXTERN xdata volatile BYTE PERI_DATA_W_L       _AT_ 0x0002;  // Data write register low byte
EXTERN xdata volatile BYTE PERI_DATA_W_H       _AT_ 0x0003;  // Data write register high byte
EXTERN xdata volatile BYTE PERI_DATA_R_L       _AT_ 0x0004;  // Data read register low byte
EXTERN xdata volatile BYTE PERI_DATA_R_H       _AT_ 0x0005;  // Data read register high byte

// USB Control Transfer Interface
EXTERN xdata volatile BYTE USB_HANDSHAKE1      _AT_ 0x0006;  // USB handshake control register 1
EXTERN xdata volatile BYTE USB_HANDSHAKE2      _AT_ 0x0007;  // USB handshake control register 2
EXTERN xdata volatile BYTE USB_CONTROL_LEN_L   _AT_ 0x0008;  // Control transfer length low byte
EXTERN xdata volatile BYTE USB_CONTROL_LEN_H   _AT_ 0x0009;  // Control transfer length high byte
EXTERN xdata volatile BYTE MCU_EXIST_REPORT    _AT_ 0x000A;  // 8051 F/W initial finish report
EXTERN xdata volatile BYTE CONTROL_OUT_DATA[8] _AT_ 0x000B;  // EP0 OUT buffer
EXTERN xdata volatile BYTE CONTROL_IN_DATA[8]  _AT_ 0x0013;  // EP0 IN buffer
EXTERN xdata volatile BYTE INTERRUPT_STATUS_L  _AT_ 0x0020;  // Status register for INT1
EXTERN xdata volatile BYTE INTERRUPT_STATUS_H  _AT_ 0x0021;  // Status register for INT1
EXTERN xdata volatile BYTE INTERRUPT_CLEAR_L   _AT_ 0x0022;  // Clear interrupt for INT1
EXTERN xdata volatile BYTE INTERRUPT_CLEAR_H   _AT_ 0x0023;  // Clear interrupt for INT1

EXTERN xdata volatile BYTE BULK_CTRL_1         _AT_ 0x0024;
EXTERN xdata volatile BYTE BULK_CTRL_2         _AT_ 0x0025;
EXTERN xdata volatile BYTE BULK_CTRL_3         _AT_ 0x0026;
EXTERN xdata volatile BYTE PERI_DEST_ADDR_H    _AT_ 0x0027;  // Destination address of peripherals high byte
EXTERN xdata volatile BYTE BULK_OUT_DATA[4]    _AT_ 0x0028;
EXTERN xdata volatile BYTE BULK_IN_DATA[4]     _AT_ 0x002C;

EXTERN xdata volatile BYTE MULTI_CH_FEEDBACK_DATA[4]     _AT_ 0x0030;
EXTERN xdata volatile BYTE SPDIF_FEEDBACK_DATA[4]     _AT_ 0x0034;
EXTERN xdata volatile BYTE TWO_CH_FEEDBACK_DATA[4]     _AT_ 0x0038;
EXTERN xdata volatile BYTE USB_HANDSHAKE3      _AT_ 0x003C;
EXTERN xdata volatile BYTE USB_HANDSHAKE4      _AT_ 0x003D;

EXTERN xdata volatile BYTE MULTI_CH_PLAYBACK_COUNT[2]    _AT_ 0x0040;
EXTERN xdata volatile BYTE SPDIF_PLAYBACK_COUNT[2]    _AT_ 0x0042;
EXTERN xdata volatile BYTE TWO_CH_PLAYBACK_COUNT[2]    _AT_ 0x0044;

#undef EXTERN
#undef _AT_

// SFR
sfr P0      = 0x80;
sfr SP      = 0x81;
sfr DPL     = 0x82;
sfr DPH     = 0x83;
sfr PCON    = 0x87;
         sbit IDLE   = 0x87+0;
         sbit STOP   = 0x87+1;
         sbit GF0    = 0x87+2;
         sbit GF1    = 0x87+3;
         sbit SMOD0  = 0x87+7;
sfr TCON    = 0x88;
         sbit IT0    = 0x88+0;
         sbit IE0    = 0x88+1;
         sbit IT1    = 0x88+2;
         sbit IE1    = 0x88+3;
         sbit TR0    = 0x88+4;
         sbit TF0    = 0x88+5;
         sbit TR1    = 0x88+6;
         sbit TF1    = 0x88+7;
sfr TMOD    = 0x89;
         sbit M00    = 0x89+0;
         sbit M10    = 0x89+1;
         sbit CT0    = 0x89+2;
         sbit GATE0  = 0x89+3;
         sbit M01    = 0x89+4;
         sbit M11    = 0x89+5;
         sbit CT1    = 0x89+6;
         sbit GATE1  = 0x89+7;
sfr TL0     = 0x8A;
sfr TL1     = 0x8B;
sfr TH0     = 0x8C;
sfr TH1     = 0x8D;
sfr P1      = 0x90;
sfr SCON    = 0x98;
         sbit RI    = 0x98+0;
         sbit TI    = 0x98+1;
         sbit RB8   = 0x98+2;
         sbit TB8   = 0x98+3;
         sbit REN   = 0x98+4;
         sbit SM2   = 0x98+5;
         sbit SM1   = 0x98+6;
         sbit SM0   = 0x98+7;
sfr SBUF    = 0x99;
sfr P2      = 0xA0;
sfr IE      = 0xA8;
         sbit EX0   = 0xA8+0;
         sbit ET0   = 0xA8+1;
         sbit EX1   = 0xA8+2;
         sbit ET1   = 0xA8+3;
         sbit ES0   = 0xA8+4;
         sbit ET2   = 0xA8+5;
         sbit ES1   = 0xA8+6;
         sbit EA    = 0xA8+7;
sfr P3      = 0xB0;
sfr IP      = 0xB8;
         sbit PX0   = 0xB8+0;
         sbit PT0   = 0xB8+1;
         sbit PX1   = 0xB8+2;
         sbit PT1   = 0xB8+3;
         sbit PS0   = 0xB8+4;
         sbit PT2   = 0xB8+5;
         sbit PS1   = 0xB8+6;
sfr PSW    = 0xD0;
         sbit P     = 0xD0+0;
         sbit FL    = 0xD0+1;
         sbit OV    = 0xD0+2;
         sbit RS0   = 0xD0+3;
         sbit RS1   = 0xD0+4;
         sbit F0    = 0xD0+5;
         sbit AC    = 0xD0+6;
         sbit CY    = 0xD0+7;
sfr ACC    = 0xE0;
sfr B      = 0xF0;
sfr MSIZ   = 0xFF;

enum {
    USB_ADDRESS = 0,             // 0x00
    EP_HALT_1,                   // 0x01
    EP_HALT_2,                   // 0x02
    EP_HALT_3,                   // 0x03
    USB_TEST_MODE,               // 0x04
    USB_CFG_RESET,               // 0x05
    REQ_REDIRECT_L,              // 0x06
    REQ_REDIRECT_H,              // 0x07

    DMA_PLAY_8CH_L,              // 0x08
    DMA_PLAY_8CH_H,              // 0x09
    DMA_PLAY_SPDIF,              // 0x0A
    DMA_PLAY_2CH,                // 0x0B

    DMA_REC_8CH_L,               // 0x0C
    DMA_REC_8CH_H,               // 0x0D
    DMA_REC_2CH,                 // 0x0E
    DMA_REC_SPDIF,               // 0x0F

    MAX_DATA_REC_8CH_L,          // 0x10
    MAX_DATA_REC_8CH_H,          // 0x11
    MAX_DATA_REC_2CH_L,          // 0x12
    MAX_DATA_REC_2CH_H,          // 0x13
    MAX_DATA_REC_SPDIF_L,        // 0x14
    MAX_DATA_REC_SPDIF_H,        // 0x15

    FEEDBACK_CTRL_L,             // 0x16
    FEEDBACK_CTRL_H,             // 0x17
    FEEDBACK_CTRL2_L,            // 0x18
    FEEDBACK_CTRL2_H,            // 0x19

    STARTING_THRESHOLD_MUTICH_L, // 0x1A
    STARTING_THRESHOLD_MUTICH_H, // 0x1B
    STARTING_THRESHOLD_SPDIF_L,  // 0x1C
    STARTING_THRESHOLD_SPDIF_H,  // 0x1D
    STARTING_THRESHOLD_2CH_L,    // 0x1E
    STARTING_THRESHOLD_2CH_H,    // 0x1F

    PLL_ADJUST_PERIOD,           // 0x20
    MISC_FUNCTION_CTRL,          // 0x21

    CHIP_PACKET_ID = 0x23,       // 0x23

    PLL_CTRL = 0x29,             // 0x29

    CLOCK_ROUTE = 0x2E,          // 0x2E

    INT_EP4_BUFFER = 0x30,       // 0x30

    INT_EP15_BUFFER = 0x40,      // 0x40

    INT_EP4_CTRL = 0x48,         // 0x48
    INT_EP15_CTRL,               // 0x49

    I2S_PLAY_8CH_L = 0x60,       // 0x60
    I2S_PLAY_8CH_H,              // 0x61
    I2S_PLAY_2CH_L,              // 0x62
    I2S_PLAY_2CH_H,              // 0x63
    I2S_PLAY_SPDIF_L,            // 0x64
    I2S_PLAY_SPDIF_H,            // 0x65

    I2S_REC_8CH_L = 0x68,        // 0x68
    I2S_REC_8CH_H,               // 0x69
    I2S_REC_2CH_L,               // 0x6A
    I2S_REC_2CH_H,               // 0x6B
    I2S_REC_SPDIF_L,             // 0x6C
    I2S_REC_SPDIF_H,             // 0x6D

    SPDIF_CTRL_0 = 0x70,         // 0x70
    SPDIF_CTRL_1,                // 0x71
    SPDIF_CTRL_2,                // 0x72
    SPDIF_CTRL_3,                // 0x73
    SPDIF_OUT0_STATUS_L,         // 0x74
    SPDIF_OUT0_STATUS_H,         // 0x75
    SPDIF_OUT1_STATUS_L,         // 0x76
    SPDIF_OUT1_STATUS_H,         // 0x77
    SPDIF_IN_STATUS_0,           // 0x78
    SPDIF_IN_STATUS_1,           // 0x79
    SPDIF_IN_STATUS_2,           // 0x7A
    SPDIF_IN_STATUS_3,           // 0x7B

    ROM_ADDR_L = 0x88,           // 0x88
    ROM_ADDR_H,                  // 0x89
    ROM_DATA,                    // 0x8A
    ROM_DUMMY_ADDR,              // 0x8B
    ROM_CTRL,                    // 0x8C

    I2C_MASTER_DEST_ADDR = 0x90, // 0x90
    I2C_MASTER_MAP_L,            // 0x91
    I2C_MASTER_MAP_H,            // 0x92
    I2C_MASTER_BUFFER,           // 0x93

    I2C_MASTER_CTRL_0 = 0x9B,    // 0x9B
    I2C_MASTER_CTRL_1,           // 0x9C

    SPI_DATA_0 = 0xA0,           // 0xA0
    SPI_DATA_1,                  // 0xA1
    SPI_DATA_2,                  // 0xA2
    SPI_CTRL_0,                  // 0xA3
    SPI_CTRL_1,                  // 0xA4
    SPI_CHIP_SELECT,             // 0xA5

    I2C_SLAVE_DATA_0 = 0xA8,     // 0xA8
    I2C_SLAVE_DATA_1,            // 0xA9
    I2C_SLAVE_MAP,               // 0xAA
    I2C_SLAVE_STATUS,            // 0xAB
    I2C_SLAVE_CTRL,              // 0xAC

    GPI_DATA = 0xB0,             // 0xB0
    GPI_INT_MASK,                // 0xB1
    GPIO_DATA_L,                 // 0xB2
    GPIO_DATA_H,                 // 0xB3
    GPIO_DIRECTION_L,            // 0xB4
    GPIO_DIRECTION_H,            // 0xB5
    GPIO_INT_MASK_L,             // 0xB6
    GPIO_INT_MASK_H,             // 0xB7
    REMOTE_WAKEUP_CTRL,          // 0xB8
    GPIO_P1_MUL_CTRL,            // 0xB9
    GPIO_P3_MUL_CTRL,            // 0xBA
    POWER_STATUS,                // 0xBB
    GPIO_DEBOUNCE_CTRL_L,        // 0xBC
    GPIO_DEBOUNCE_CTRL_H,        // 0xBD
    GPI_DEBOUNCE_CTRL,           // 0xBE

    PLAYBACK_ROUTING_L = 0xC0,   // 0xC0
    PLAYBACK_ROUTING_H,          // 0xC1
    RECORD_ROUTING_L,            // 0xC2
    RECORD_ROUTING_H,            // 0xC3
    PLAYBACK_MUTE,               // 0xC4
    RECORD_MUTE,                 // 0xC5
    ADC_MONITOR_CTRL,            // 0xC6
    MULTI_CH_RECORD_ROUTING,     // 0xC7

    HDA_STATUS_CTRL_L = 0xD0,    // 0xD0
    HDA_STATUS_CTRL_H,           // 0xD1
    HDA_PLAYBACK_DMA1_CTRL,      // 0xD2
    HDA_PLAYBACK_DMA2_CTRL,      // 0xD3
    HDA_PLAYBACK_DMA3_CTRL,      // 0xD4
    HDA_RECORD_DMA1_CTRL,        // 0xD5
    HDA_RECORD_DMA2_CTRL,        // 0xD6
    HDA_RECORD_DMA3_CTRL,        // 0xD7
    HDA_VERB_RESPONSE_0,         // 0xD8
    HDA_VERB_RESPONSE_1,         // 0xD9
    HDA_VERB_RESPONSE_2,         // 0xDA
    HDA_VERB_RESPONSE_3,         // 0xDB
    HDA_PLAYBACK_ID_L,           // 0xDC
    HDA_PLAYBACK_ID_H,           // 0xDD
    HDA_RECORD_ID_L,             // 0xDE
    HDA_RECORD_ID_H,             // 0xDF

    MULTI_CH_FIFO_REMAIN_L,      // 0xE0
    MULTI_CH_FIFO_REMAIN_H,      // 0xE1
    SPDIF_FIFO_REMAIN_L,         // 0xE2
    SPDIF_FIFO_REMAIN_H,         // 0xE3
    TWO_CH_FIFO_REMAIN_L,        // 0xE4
    TWO_CH_FIFO_REMAIN_H,        // 0xE5
    DMA_FIFO_FLUSH,              // 0xE6
    DMA_FIFO_MUTE,               // 0xE7
    PLL_THRES_LIMIT_L,           // 0xE8
    PLL_THRES_LIMIT_H,           // 0xE9
    FEEDBACK_CTRL3,              // 0xEA

    PAD_GP10_GP11_CTRL = 0xF5,   // 0xF5
    PAD_GP12_GP13_CTRL,          // 0xF6

    PAD_GP22_GP23_CTRL = 0xFB,   // 0xFB
    PAD_GP24_GP25_CTRL,          // 0xFC

    OSC_CTRL = 0x110             // 0x110
};

#define DMA_32000    0x00
#define DMA_44100    0x08
#define DMA_48000    0x10
#define DMA_64000    0x18
#define DMA_88200    0x20
#define DMA_96000    0x28
#define DMA_176400   0x30
#define DMA_192000   0x38
#define DMA_352800   0xB0
#define DMA_384000   0xB8
#define DMA_705600   0xB1
#define DMA_768000   0xB9
#define DMA_1411200  0xB2
#define DMA_1536000  0xBA
#define DMA_8000     0x40
#define DMA_16000    0x48
#define FREQ_MASK    0x78

#define SPDIF_CTRL_32000  0x00
#define SPDIF_CTRL_48000  0x01
#define SPDIF_CTRL_96000  0x02
#define SPDIF_CTRL_192000 0x03
#define SPDIF_CTRL_44100  0x04
#define SPDIF_CTRL_88200  0x06
#define SPDIF_CTRL_176400 0x07

#define SPDIF_STATUS_44100  0x00
#define SPDIF_STATUS_48000  0x02
#define SPDIF_STATUS_32000  0x03
#define SPDIF_STATUS_88200  0x08
#define SPDIF_STATUS_96000  0x0A
#define SPDIF_STATUS_64000  0x0B
#define SPDIF_STATUS_176400 0x0C
#define SPDIF_STATUS_192000 0x0E

#define MCLK_TRI_STATE 0x2000
#define MCLK_MUTE 0x1000

#define BCLK_LRCK_64   0x0000
#define BCLK_LRCK_128  0x0400
#define BCLK_LRCK_256  0x0800

#define I2S_16Bit  0x0000
#define I2S_20Bit  0x0100
#define I2S_24Bit  0x0200
#define I2S_32Bit  0x0300

#define I2S_MASTER 0x0080
#define I2S_SLAVE  0x0000

#define MCLK_LRCK_128  0x0000
#define MCLK_LRCK_256  0x0020
#define MCLK_LRCK_512  0x0040

#define I2S_MODE   0x0000
#define LEFT_JUST  0x0010

#define I2S_32000  0x0000
#define I2S_44100  0x0001
#define I2S_48000  0x0002
#define I2S_64000  0x0003
#define I2S_88200  0x0004
#define I2S_96000  0x0005
#define I2S_176400 0x0006
#define I2S_192000 0x0007
#define I2S_8000   0x0008
#define I2S_16000  0x0009

#define DMA_2CH    0x00
#define DMA_4CH    0x01
#define DMA_6CH    0x02
#define DMA_8CH    0x03

#define DMA_16Bit  0x00
#define DMA_24Bit_32Container  0x02
#define DMA_32Bit  0x04
#define DMA_24Bit  0x06 // True 24bits
#define RESOLUTION_MASK 0x06

#define DMA_START  0x01
#define DMA_STOP   0x00
#define NON_PCM    0x80

#define HDA_2CH    0x10
#define HDA_4CH    0x30
#define HDA_6CH    0x50
#define HDA_8CH    0x70

#define HDA_16Bit  0x00
#define HDA_20Bit  0x01
#define HDA_24Bit  0x02
#define HDA_32Bit  0x03

#define HDA_44100  0x80
#define HDA_48000  0x00
#define HDA_88200  0x84
#define HDA_96000  0x04
#define HDA_176400 0x8C
#define HDA_192000 0x0C

#define HDA_CODEC_2CH 0x01
#define HDA_CODEC_4CH 0x03
#define HDA_CODEC_6CH 0x05
#define HDA_CODEC_8CH 0x07

#define HDA_CODEC_16Bit 0x10
#define HDA_CODEC_24Bit 0x30
#define HDA_CODEC_32Bit 0x40

#define HDA_CODEC_44100  0x4000
#define HDA_CODEC_48000  0x0000
#define HDA_CODEC_88200  0x4800
#define HDA_CODEC_96000  0x0800
#define HDA_CODEC_176400 0x5800
#define HDA_CODEC_192000 0x1800

#define EP_BULK_OUT 0x01
#define EP_BULK_IN 0x82
#define EP_HID_INT 0x84
#define EP_MULTI_CH_PLAY 0x05
#define EP_SPDIF_PLAY 0x06
#define EP_TWO_CH_PLAY 0x07
#define EP_MULTI_CH_REC 0x88
#define EP_TWO_CH_REC 0x89
#define EP_SPDIF_REC 0x8A
#define EP_AUDIO_INT 0x8F

#define MULTI_CH_PLAY_ROUTE_I2S 0x00
#define MULTI_CH_PLAY_ROUTE_HDA 0x10
#define TWO_CH_PLAY_ROUTE_I2S 0x00
#define TWO_CH_PLAY_ROUTE_HDA 0x20
#define SPDIF_PLAY_ROUTE_SPDIF 0x00
#define SPDIF_PLAY_ROUTE_HDA 0x40
#define SPDIF_PLAY_ROUTE_I2S 0x80

#define MULTI_CH_REC_ROUTE_I2S 0x00
#define MULTI_CH_REC_ROUTE_HDA 0x01
#define TWO_CH_REC_ROUTE_I2S 0x00
#define TWO_CH_REC_ROUTE_HDA 0x02
#define SPDIF_REC_ROUTE_SPDIF 0x00
#define SPDIF_REC_ROUTE_HDA 0x04
#define SPDIF_REC_ROUTE_I2S 0x08

#define USE_XSPDIFI0 0x00
#define USE_XSPDIFI1 0x20

#define BULK_OUT_MAX_PACKET_SIZE_16	0x00
#define BULK_OUT_MAX_PACKET_SIZE_32	0x04
#define BULK_OUT_MAX_PACKET_SIZE_64	0x08
#define BULK_OUT_MAX_PACKET_SIZE_512	0x0C

#define BULK_IN_MAX_PACKET_SIZE_16	0x00
#define BULK_IN_MAX_PACKET_SIZE_32	0x20
#define BULK_IN_MAX_PACKET_SIZE_64	0x40
#define BULK_IN_MAX_PACKET_SIZE_512	0x60

#endif

