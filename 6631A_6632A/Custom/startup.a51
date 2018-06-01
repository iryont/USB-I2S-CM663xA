#include "config.h"

$NOMOD51

;********************************************************************
; SFR Symbols 
;********************************************************************
	SP DATA 81H

;********************************************************************
; User Defined Macro
;********************************************************************
	MCU_EXIST_REPORT EQU 000AH	
	DATALEN EQU 256	

;********************************************************************
; Data Allocation
;********************************************************************
	?STACK SEGMENT DATA
	RSEG ?STACK
	DS 20H  

	NAME ?C_STARTUP
	PUBLIC ?C_STARTUP
	EXTRN CODE (?C_START)

	CSEG AT 2000H
	LJMP ?C_STARTUP

	CSEG AT 2026H
	DS 02H

;********************************************************************
; Start up code
;********************************************************************
	CSEG AT 2028H		; Reserve interrupt vetor table

?C_STARTUP:
	;; Notify MCU exist
	MOV DPTR, #MCU_EXIST_REPORT
#ifdef _FULL_SPEED_ONLY_
	MOV A, #03H
#else
	MOV A, #01H
#endif
	MOVX @DPTR, A

	;; Clear MCU memory
	MOV R0, #DATALEN - 1
	CLR A
IDATALOOP:
	MOV @R0, A
	DJNZ R0, IDATALOOP

	;; Setup stack
	MOV SP, #?STACK-1

	LJMP ?C_START

	END

