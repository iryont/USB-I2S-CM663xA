set opath=%path%
path=c:\keil\c51\bin;%path%

set C51INC=c:\keil\c51\inc
set C51LIB=c:\keil\c51\lib

set CONFIG=

if "%2" neq "" set CONFIG=%CONFIG%%2
if "%3" neq "" set CONFIG=%CONFIG%,%3
if "%4" neq "" set CONFIG=%CONFIG%,%4

set OBJ_FILES=audio.obj,audio10.obj,audio20.obj,device.obj,startup.obj,dscr.obj,timer.obj,uart.obj,hid.obj,usb.obj,request.obj,peripheral.obj
set INCLUDE_PATH=..\Common\Driver;..\Common\Usb;..\Common\Hid;.\Custom;.\Audio

if "%CONFIG%" equ "" (
    set A51FLAGS="NOPRINT SET(SMALL) EP"
    set C51FLAGS="OMF2 NOPRINT OPTIMIZE(9,SPEED) INCDIR(%INCLUDE_PATH%) INTVECTOR(0x2000)"
) else (
    set A51FLAGS="NOPRINT SET(SMALL) EP DEFINE(%CONFIG%)"
    set C51FLAGS="OMF2 NOPRINT OPTIMIZE(9,SPEED) INCDIR(%INCLUDE_PATH%) INTVECTOR(0x2000) DEFINE(%CONFIG%)"
)

set LX51FLAGS=CLASSES(DATA(D:0x00-D:0x7F),IDATA(I:0x80-I:0xFF),CODE(C:0x2000-C:0x7FFF),CONST(C:0x2000-C:0x7FFF)) SEGMENTS(?STACK(D:0x60))

call ..\Common\Driver\make
call ..\Common\Usb\make
call ..\Common\Hid\make
call Audio\make
call Custom\make

lx51 %OBJ_FILES% to %1 %LX51FLAGS%
ohx51 %1 HEX
del *.obj

:exit
set CONFIG=
set C51INC=
set C51LIB=
set A51FLAGS=
set C51FLAGS=
set BL51FLAGS=
set OBJ_FILES=
set path=%opath%
set opath=
