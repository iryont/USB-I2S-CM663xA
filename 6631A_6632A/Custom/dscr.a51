;;-----------------------------------------------------------------------------
;;   File:      dscr.a51
;;   Contents:   This file contains descriptor data tables.  
;;
;;   Copyright (c) 2003 Cypress Semiconductor, Inc. All rights reserved
;;-----------------------------------------------------------------------------
#include "config.h" 

DSCR_DEVICE           	equ    1   ;; Descriptor type: Device
DSCR_CONFIG           	equ    2   ;; Descriptor type: Configuration
DSCR_STRING           	equ    3   ;; Descriptor type: String
DSCR_INTRFC           	equ    4   ;; Descriptor type: Interface
DSCR_ENDPNT           	equ    5   ;; Descriptor type: Endpoint
DSCR_DEVQUAL          	equ    6   ;; Descriptor type: Device Qualifier
DSCR_INTRFC_ASSOC  equ    11  	   ;; Descriptor type: Interface Association

DSCR_CS_INTERFACE   equ    24H
DSCR_CS_ENDPOINT    equ    25H

AC_SUBTYPE_HEADER   equ    1
AC_SUBTYPE_IT         	equ    2
AC_SUBTYPE_OT         	equ    3
AC_SUBTYPE_MIXER      equ    4
AC_SUBTYPE_SELECTOR   equ    5
AC_SUBTYPE_FEATURE    equ    6
AC_SUBTYPE_EXTENSION  equ    9
AC_SUBTYPE_CLKSRC     equ    10
AC_SUBTYPE_CLKSEL     equ    11
AC_SUBTYPE_CLKMUL     equ    12

AS_SUBTYPE_GENERAL    equ    1
AS_SUBTYPE_FORMAT     equ    2
EP_SUBTYPE_GENERAL    equ    1

DSCR_AC_IT_LEN   equ   12
DSCR_AC_OT_LEN   equ   9
DSCR_AS_GENERAL_LEN  equ   7
DSCR_EP_GENERAL_LEN  equ   7
DSCR_AC_IT_LEN_20        equ    17
DSCR_AC_OT_LEN_20       equ    12
DSCR_AS_GENERAL_LEN_20   equ    16
DSCR_EP_GENERAL_LEN_20   equ    8


DSCR_INTRFC_ASSOC_LEN equ    8
DSCR_AC_HDR_LEN       equ    9
DSCR_AC_CLKSRC_LEN    equ    8
DSCR_AC_CLKMUL_LEN    equ    7
DSCR_AS_FORMAT_LEN    equ    6
DSCR_DEVICE_LEN       equ    18
DSCR_CONFIG_LEN       equ    9
DSCR_INTRFC_LEN       equ    9
DSCR_ENDPNT_LEN       equ    7
DSCR_DEVQUAL_LEN      equ    10

ET_CONTROL            equ    0   ;; Endpoint type: Control
ET_ISO                equ    1   ;; Endpoint type: Isochronous
ET_BULK               equ    2   ;; Endpoint type: Bulk
ET_INT                equ    3   ;; Endpoint type: Interrupt

;;-----------------------------------------------------------------------------
;; Global Variables
;;-----------------------------------------------------------------------------
public Audio20DeviceDscr, Audio20DeviceQualDscr, Audio20HighSpeedConfigDscr, Audio20FullSpeedConfigDscr
public Audio10DeviceDscr, Audio10DeviceQualDscr, Audio10HighSpeedConfigDscr, Audio10FullSpeedConfigDscr
public StringDscr, HidDscr, HidReportDscr, HidReportDscrLen 

USB_DESCRIPTOR SEGMENT CODE
RSEG USB_DESCRIPTOR

Audio20DeviceDscr:
      db   DSCR_DEVICE_LEN   ;; Descriptor length
      db   DSCR_DEVICE       ;; Decriptor type
      dw   0002H             ;; Specification Version (BCD)
      db   0EFH              ;; Device class
      db   2                 ;; Device sub-class
      db   1                 ;; Device sub-sub-class
      db   64                ;; Maximum packet size
      dw   VENDOR_ID         ;; Vendor ID
      dw   PRODUCT_ID        ;; Product ID
      dw   VERSION_ID        ;; Product version ID
      db   1                 ;; Manufacturer string index
      db   2                 ;; Product string index
      db   0                 ;; Serial number string index
      db   1                 ;; Number of configurations

Audio20DeviceQualDscr:
      db   DSCR_DEVQUAL_LEN  ;; Descriptor length
      db   DSCR_DEVQUAL      ;; Decriptor type
      dw   0002H             ;; Specification Version (BCD)
      db   0EFH              ;; Device class
      db   2                 ;; Device sub-class
      db   1                 ;; Device protocol
      db   64                ;; Maximum packet size
      db   1                 ;; Number of configurations
      db   0                 ;; Reserved

#include "Audio20HighSpeed.conf"

#include "Audio20FullSpeed.conf"

Audio10DeviceDscr:   
      db   DSCR_DEVICE_LEN   ;; Descriptor length
      db   DSCR_DEVICE       ;; Decriptor type
      dw   0002H             ;; Specification Version (BCD)
      db   00H               ;; Device class
      db   00H               ;; Device sub-class
      db   00H               ;; Device sub-sub-class
      db   64                ;; Maximum packet size
      dw   VENDOR_ID         ;; Vendor ID
      dw   PRODUCT_ID+16     ;; Product ID
      dw   VERSION_ID        ;; Product version ID
      db   1                 ;; Manufacturer string index
      db   2                 ;; Product string index
      db   0                 ;; Serial number string index
      db   1                 ;; Number of configurations

Audio10DeviceQualDscr:
      db   DSCR_DEVQUAL_LEN  ;; Descriptor length
      db   DSCR_DEVQUAL      ;; Decriptor type
      dw   0002H             ;; Specification Version (BCD)
      db   00H               ;; Device class
      db   00H               ;; Device sub-class
      db   00H               ;; Device protocol
      db   64                ;; Maximum packet size
      db   1                 ;; Number of configurations
      db   0                 ;; Reserved

#include "Audio10HighSpeed.conf"

#include "Audio10FullSpeed.conf"

StringDscr:

      db   0
StringDscr0:   
      db   StringDscr0End-StringDscr0
      db   DSCR_STRING
      db   09H,04H
StringDscr0End:

      db   1
StringDscr1:   
      db   StringDscr1End-StringDscr1
      db   DSCR_STRING
      db   'K',00
      db   'u',00
      db   'S',00
      db   'y',00
      db   'A',00
      db   'u',00
      db   'd',00
      db   'i',00
      db   'o',00
StringDscr1End:

      db   2
StringDscr2:   
      db   StringDscr2End-StringDscr2
      db   DSCR_STRING
      db   'K',00
      db   'u',00
      db   'S',00
      db   'y',00
      db   ' ',00
      db   'A',00
      db   'u',00
      db   'd',00
      db   'i',00
      db   'o',00
      db   ' ',00
      db   '2',00
      db   '.',00
      db   '0',00
StringDscr2End:

      db   3
StringDscr3:   
      db   StringDscr3End-StringDscr3
      db   DSCR_STRING
      db   'K',00
      db   'u',00
      db   'S',00
      db   'y',00
      db   ' ',00
      db   'A',00
      db   'u',00
      db   'd',00
      db   'i',00
      db   'o',00
      db   ' ',00
      db   '1',00
      db   '.',00
      db   '0',00
StringDscr3End:

      db   4
StringDscr4:   
      db   StringDscr4End-StringDscr4
      db   DSCR_STRING
      db   'D',00
      db   'i',00
      db   'g',00
      db   'i',00
      db   't',00
      db   'a',00
      db   'l',00
StringDscr4End:

HidReportDscr:
	db	05H, 0CH			;; Usage Page(Consumer) 		
	db	09H, 01H			;; Usage(Consumer Control)
	
	db	0A1H, 01H		;; Collection(Application)
	
	db	15H, 00H			;; Logical Minimum(0)
	db	25H, 01H			;; Logical Maximum(1)

	db	09H, 0E9H		;; Usage(Volume Increment)
	db	09H, 0EAH		;; Usage(Volume Decrement)
	db	75H, 01H			;; Report Size(1)		
	db	95H, 02H			;; Report Count(2)
	db	81H, 02H			;; Input(Data, Variable, Absolute)

	db	09H, 0E2H		;; Usage(Mute)
	db	09H, 00H			;; Usage(Unassigned)
	db	81H, 06H			;; Input(Data, Variable, Relative)   

	db	09H, 00H			;; Usage(Unassigned)
	db	95H, 04H			;; Report Count(4)
	db	81H, 02H			;; Input(Data, Variable, Absolute)  

	db	26H, 0FFH, 00H	;; Logical Maximun(255)

	db	09H, 00H			;; Usage(Unassigned)
	db	75H, 08H			;; Report Size(8)
	db	95H, 0FH			;; Report Count(15)
	db	81H, 02H			;; Input(Data, Variable, Absolute)

	db	09H, 00H			;; Usage(Unassigned)
	db	95H, 10H			;; Report Count(16)
	db	91H, 02H			;; Output(Data, Variable, Absolute)

	db	0C0H			;; End Collection

HidReportDscrEnd:

      end

