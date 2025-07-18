"""Webots launcher"""

# MIT License
#
# Copyright (c) 2022 - 2025 Andreas Merkle (web@blue-andi.de)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

################################################################################
# Imports
################################################################################
import os
import sys
import platform

Import("env") # pylint: disable=undefined-variable

################################################################################
# Variables
################################################################################
OS_PLATFORM_TYPE_WIN = "Windows"
OS_PLATFORM_TYPE_LINUX = "Linux"
OS_PLATFORM_TYPE_MACOS = "Darwin"
OS_PLATFORM_TYPE = platform.system()
ROBOT_NAME = env.GetProjectOption("webots_robot_name") # pylint: disable=undefined-variable
ROBOT_SERIAL_RX_CHANNEL = env.GetProjectOption("webots_robot_serial_rx_channel") # pylint: disable=undefined-variable
ROBOT_SERIAL_TX_CHANNEL = env.GetProjectOption("webots_robot_serial_tx_channel") # pylint: disable=undefined-variable
PROGRAM_PATH = "$BUILD_DIR/"
PROGRAM_OPTIONS = ''
PROGRAM_OPTIONS_ZUMO_COM_SYSTEM = '-c ' \
                                + '--serialRxCh ' + ROBOT_SERIAL_RX_CHANNEL + ' ' \
                                + '--serialTxCh ' + ROBOT_SERIAL_TX_CHANNEL + ' ' \
                                + '-v'
WEBOTS_CONTROLLER_OPTIONS = '--robot-name=' + ROBOT_NAME + ' --stdout-redirect'
WEBOTS_HOME = os.getenv('WEBOTS_HOME')

if OS_PLATFORM_TYPE == OS_PLATFORM_TYPE_WIN:

    WEBOTS_HOME = WEBOTS_HOME.replace('\\', '/')
    WEBOTS_CONTROLLER  = "\""
    WEBOTS_CONTROLLER += f"{WEBOTS_HOME}/msys64/mingw64/bin/webots-controller.exe"
    WEBOTS_CONTROLLER += "\""
    PROGRAM_NAME = "${PROGNAME}.exe"

elif OS_PLATFORM_TYPE == OS_PLATFORM_TYPE_LINUX:

    WEBOTS_CONTROLLER = f"{WEBOTS_HOME}/webots-controller"
    PROGRAM_NAME = "${PROGNAME}"

elif OS_PLATFORM_TYPE == OS_PLATFORM_TYPE_MACOS:

    WEBOTS_CONTROLLER = f"{WEBOTS_HOME}/Contents/MacOS/webots-controller"
    PROGRAM_NAME = "${PROGNAME}"

else:
    print(f"OS type {OS_PLATFORM_TYPE} not supported.")
    sys.exit(1)

WEBOTS_LAUNCHER_ACTION = WEBOTS_CONTROLLER + ' '\
                        + WEBOTS_CONTROLLER_OPTIONS + ' ' \
                        + PROGRAM_PATH + PROGRAM_NAME + ' ' \
                        + PROGRAM_OPTIONS

WEBOTS_LAUNCHER_ZUMO_COM_SYSTEM_ACTION = WEBOTS_CONTROLLER + ' ' \
                                        + WEBOTS_CONTROLLER_OPTIONS + ' ' \
                                        + PROGRAM_PATH + PROGRAM_NAME + ' ' \
                                        + PROGRAM_OPTIONS_ZUMO_COM_SYSTEM

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################

# pylint: disable=undefined-variable
env.AddCustomTarget(
    name="webots_launcher",
    dependencies=PROGRAM_PATH + PROGRAM_NAME,
    actions=[
        WEBOTS_LAUNCHER_ACTION
    ],
    title="Launch alone",
    description="Launch application with Webots launcher and without ZumoComSystem."
)

# pylint: disable=undefined-variable
env.AddCustomTarget(
    name="webots_launcher_zumo_com_system",
    dependencies=PROGRAM_PATH + PROGRAM_NAME,
    actions=[
        WEBOTS_LAUNCHER_ZUMO_COM_SYSTEM_ACTION
    ],
    title="Launch with ZumoComSystem",
    description="Launch application with Webots launcher and with ZumoComSystem."
)
