@echo off
set PIO_PATH=%USERPROFILE%\.platformio\packages\toolchain-atmelavr\bin
set OBJDUMP=%PIO_PATH%\avr-objdump.exe
set ENV=ConvoyLeaderTarget
set SRC=.pio\build\%ENV%\firmware.elf
set DST=firmware.lst

@echo Create list file of %ENV% to %DST%
%OBJDUMP% -C -d -S %SRC% > %DST%
@echo Ready
pause
