"""Webots launcher"""

# MIT License
#
# Copyright (c) 2022 - 2024 Andreas Merkle (web@blue-andi.de)
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

Import("env") # pylint: disable=undefined-variable

################################################################################
# Variables
################################################################################
OS_TYPE = env["PLATFORM"] # pylint: disable=undefined-variable
FILE_EXTENSION = ".exe" if OS_TYPE == "win32" else ""
WEBOTS_HOME = os.getenv('WEBOTS_HOME').replace('\\', '/')
WEBOTS_CONTROLLER = '"' + WEBOTS_HOME + '/msys64/mingw64/bin/webots-controller"' + FILE_EXTENSION
ROBOT_NAME = env.GetProjectOption("webots_robot_name") # pylint: disable=undefined-variable
WEBOTS_CONTROLLER_OPTIONS = '--robot-name=' + ROBOT_NAME + ' --stdout-redirect'
PROGRAM = "$BUILD_DIR/${PROGNAME}" + FILE_EXTENSION
PROGRAM_OPTIONS_SOCKET = '-s'

WEBOTS_LAUNCHER_ACTION = WEBOTS_CONTROLLER + ' '\
                        + WEBOTS_CONTROLLER_OPTIONS + ' ' \
                        + PROGRAM

WEBOTS_LAUNCHER_SOCKET_ACTION = WEBOTS_CONTROLLER + ' ' \
                                + WEBOTS_CONTROLLER_OPTIONS + ' ' \
                                + PROGRAM + ' ' \
                                + PROGRAM_OPTIONS_SOCKET

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
    dependencies=PROGRAM,
    actions=[
        WEBOTS_LAUNCHER_ACTION
    ],
    title="WebotsLauncher",
    description="Launch application with Webots launcher."
)

# pylint: disable=undefined-variable
env.AddCustomTarget(
    name="webots_launcher_socket",
    dependencies=PROGRAM,
    actions=[
        WEBOTS_LAUNCHER_SOCKET_ACTION
    ],
    title="WebotsLauncherSocket",
    description="Launch application with Webots launcher and enable socket."
)
