@echo off
set PIO_PATH=%USERPROFILE%\.platformio\packages\toolchain-atmelavr\bin
set NM=%PIO_PATH%\avr-nm.exe
set ENV=ConvoyLeaderTarget
set SRC=.pio\build\%ENV%\firmware.elf
set DST=firmware.sym

@echo Create symbol file of %ENV% to %DST%
%NM% -C -S --size-sort %SRC% > %DST%
@echo Ready
pause
