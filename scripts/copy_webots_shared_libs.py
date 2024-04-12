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

Import("env") # type: ignore

################################################################################
# Variables
################################################################################

################################################################################
# Variables
################################################################################

# 2024_02_27 eble: Make this script platform independant since
# There are no *.dll on Linux or on Macs
WINDOWS_PLATFORM_NAME = 'Windows'
MACOS_PLATFORM_NAME = 'Darwin'
LINUX_PLATFORM_NAME = 'Linux'

SHARED_FILES_LIST = []

if platform.system() == WINDOWS_PLATFORM_NAME:
    SHARED_FILES_LIST = [
    '/lib/controller/Controller.dll',
    '/lib/controller/CppController.dll']
elif platform.system() == MACOS_PLATFORM_NAME:
        SHARED_FILES_LIST = [
    '/lib/controller/libController.dylib',
    '/lib/controller/libCppController.dylib']
elif platform.system() == LINUX_PLATFORM_NAME:
    SHARED_FILES_LIST = [
    '/lib/controller/libController.so',
    '/lib/controller/libCppController.so']
else:
    print("I don't know your platform, it's neither Windows nor Linux nor Mac. Exiting script.")
    sys.exit(1)



PIO_ENV_NAME = env["PIOENV"] # type: ignore

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
