@echo off
setlocal

rem Change working directory to the directory of the batch script.
cd /d %~dp0

set WEBOTS_CONTROLLER="%WEBOTS_HOME%\msys64\mingw64\bin\webots-controller.exe"
set PROGRAM_NAME=program.exe
set CONVOY_LEADER_RX_CHANNEL=1
set CONVOY_LEADER_TX_CHANNEL=2

set CONVOY_LEADER_PIO_ENV_NAME=ConvoyLeaderSim
set CONVOY_LEADER_PIO_PATH=.pio\build\%CONVOY_LEADER_PIO_ENV_NAME%
set CONVOY_LEADER_ROBOT_NAME=leader
set CONVOY_LEADER_RX_CHANNEL=1
set CONVOY_LEADER_TX_CHANNEL=2

set CONVOY_FOLLOWER_PIO_ENV_NAME=ConvoyFollowerSim
set CONVOY_FOLLOWER_PIO_PATH=.pio\build\%CONVOY_FOLLOWER_PIO_ENV_NAME%
set CONVOY_FOLLOWER_ROBOT_NAME=follower_

set TEMP_PLATOON_PATH=tmp
set TEMP_PLATOON_LEADER_PATH=%TEMP_PLATOON_PATH%\%CONVOY_LEADER_PIO_ENV_NAME%
set TEMP_PLATOON_LEADER_NEW_PATH=%TEMP_PLATOON_PATH%\%CONVOY_LEADER_ROBOT_NAME%
set TEMP_PLATOON_FOLLOWER_PATH=%TEMP_PLATOON_PATH%\%CONVOY_FOLLOWER_PIO_ENV_NAME%
set TEMP_PLATOON_FOLLOWER_NEW_PATH=%TEMP_PLATOON_PATH%\%CONVOY_FOLLOWER_ROBOT_NAME%

rem Compile Convoy Leader
%USERPROFILE%\.platformio\penv\Scripts\pio.exe run --environment %CONVOY_LEADER_PIO_ENV_NAME%

rem Compile Convoy Follower
%USERPROFILE%\.platformio\penv\Scripts\pio.exe run --environment %CONVOY_FOLLOWER_PIO_ENV_NAME%

rem If temporary folder doesn't exist, it will be created.
if not exist %TEMP_PLATOON_PATH%\ (
    md %TEMP_PLATOON_PATH%
)

rem Unzip leader to temporary folder
if exist %TEMP_PLATOON_LEADER_PATH%\ (
    rmdir /s /q %TEMP_PLATOON_LEADER_PATH%\
)
Call :UnZipFile "%~dp0%TEMP_PLATOON_PATH%\" "%~dp0%CONVOY_LEADER_PIO_PATH%\%CONVOY_LEADER_PIO_ENV_NAME%.zip"

rem Unzip follower to temporary folder
if exist %TEMP_PLATOON_FOLLOWER_PATH%\ (
    rmdir /s /q %TEMP_PLATOON_FOLLOWER_PATH%\
)
Call :UnZipFile "%~dp0%TEMP_PLATOON_PATH%\" "%~dp0%CONVOY_FOLLOWER_PIO_PATH%\%CONVOY_FOLLOWER_PIO_ENV_NAME%.zip"

rem Copy leader folder according to a new folder with the robot's name.
rem The copy ensures that a existing settings.json will be kept.
xcopy /s /y %TEMP_PLATOON_LEADER_PATH%\ %TEMP_PLATOON_LEADER_NEW_PATH%\

rem Copy follower folder according to a new folder with the robot's name.
rem The copy ensures that a existing settings.json will be kept.
for /L %%i in (1, 1, 10) do (
    xcopy /s /y %TEMP_PLATOON_FOLLOWER_PATH%\ %TEMP_PLATOON_FOLLOWER_NEW_PATH%%%i\
)

rem Delete extraced files from .zip
rmdir /s /q %TEMP_PLATOON_LEADER_PATH%
rmdir /s /q %TEMP_PLATOON_FOLLOWER_PATH%

rem Start the convoy leader
echo Start convoy leader.
start "Convoy Leader" ""%WEBOTS_CONTROLLER%"" --robot-name=%CONVOY_LEADER_ROBOT_NAME% --stdout-redirect %TEMP_PLATOON_LEADER_NEW_PATH%\%PROGRAM_NAME% -n %CONVOY_LEADER_ROBOT_NAME% -c --serialRxCh=%CONVOY_LEADER_RX_CHANNEL% --serialTxCh=%CONVOY_LEADER_TX_CHANNEL% -v"

rem Start the followers
for /L %%i in (1, 1, 10) do (
    echo Start convoy follower %%i.
    call :StartFollower %%i
)

exit /b

:StartFollower
set INSTANCE=%1
set ROBOT_NAME=%CONVOY_FOLLOWER_ROBOT_NAME%%instance%
set /a SERIAL_RX_CHANNEL=(INSTANCE * 2) + 1
set /a SERIAL_TX_CHANNEL=(INSTANCE * 2) + 2
echo Start convoy follower %INSTANCE%.
start "Convoy Follower %INSTANCE%" ""%WEBOTS_CONTROLLER%"" --robot-name=%ROBOT_NAME% --stdout-redirect %TEMP_PLATOON_FOLLOWER_NEW_PATH%%INSTANCE%\%PROGRAM_NAME%  -n %ROBOT_NAME% -c --serialRxCh=%SERIAL_RX_CHANNEL% --serialTxCh=%SERIAL_TX_CHANNEL% -v"
exit /b

:UnZipFile <extractTo> <newzipfile>
SET vbs="%TEMP%\_.vbs"
IF EXIST %vbs% DEL /f /q %vbs%
>%vbs% ECHO Set fso = CreateObject("Scripting.FileSystemObject")
>>%vbs% ECHO If NOT fso.FolderExists(%1) Then
>>%vbs% ECHO fso.CreateFolder(%1)
>>%vbs% ECHO End If
>>%vbs% ECHO set objShell = CreateObject("Shell.Application")
>>%vbs% ECHO set FilesInZip=objShell.NameSpace(%2).items
>>%vbs% ECHO objShell.NameSpace(%1).CopyHere(FilesInZip)
>>%vbs% ECHO Set fso = Nothing
>>%vbs% ECHO Set objShell = Nothing
cscript //nologo %vbs%
IF EXIST %vbs% DEL /f /q %vbs%
