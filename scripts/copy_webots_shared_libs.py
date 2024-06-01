"""Copies the Webots shared libraries."""

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
import shutil
import os
import platform
import sys

Import("env") # pylint: disable=undefined-variable

################################################################################
# Variables
################################################################################

################################################################################
# Variables
################################################################################

OS_PLATFORM_TYPE_WIN = "Windows"
OS_PLATFORM_TYPE_LINUX = "Linux"
OS_PLATFORM_TYPE_MACOS = "Darwin"
OS_PLATFORM_TYPE = platform.system()

SHARED_FILES_LIST = []

if OS_PLATFORM_TYPE == OS_PLATFORM_TYPE_WIN:

    SHARED_FILES_LIST = [
        '/lib/controller/Controller.dll',
        '/lib/controller/CppController.dll'
    ]

elif OS_PLATFORM_TYPE == OS_PLATFORM_TYPE_MACOS:

    SHARED_FILES_LIST = [
        '/lib/controller/libController.dylib',
        '/lib/controller/libCppController.dylib'
    ]

elif OS_PLATFORM_TYPE == OS_PLATFORM_TYPE_LINUX:

    SHARED_FILES_LIST = [
        '/lib/controller/libController.so',
        '/lib/controller/libCppController.so'
    ]

else:
    print(f"OS type {OS_PLATFORM_TYPE} not supported.")
    sys.exit(1)

PIO_ENV_NAME = env["PIOENV"] # pylint: disable=undefined-variable

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################

for filePath in SHARED_FILES_LIST:
    fullPath = os.getenv('WEBOTS_HOME') + filePath
    shutil.copy(fullPath, './.pio/build/' + PIO_ENV_NAME)
