@echo off

if "%1" == "" (
    echo [Usage] : %0 {Version}
    echo Example : %0 0921
    goto exit
)

if exist stdout.txt del stdout.txt

set BOOT_PATH=..\Common\Boot

set OUTPUT=6631A-%1
set VID_PID=0D8C 0004
if exist %OUTPUT% del/q %OUTPUT%*
call make %OUTPUT% 1>>stdout.txt
if exist %OUTPUT% (
    %BOOT_PATH%\AddInfo %OUTPUT%.hex %BOOT_PATH%\CM66xxBoot.hex 6631 %1 %VID_PID% 1>>stdout.txt
) else (
    echo Build %OUTPUT% Fail
)

set BOOT_PATH=

:exit

@echo on
