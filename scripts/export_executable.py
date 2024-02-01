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
import os
from zipfile import ZipFile

Import("env")  # type: ignore

################################################################################
# Variables
################################################################################

PIO_ENV_NAME = env["PIOENV"]  # type: ignore
WORKDIR_PATH = './.pio/build/' + PIO_ENV_NAME
OUT_PATH = WORKDIR_PATH + "/" + PIO_ENV_NAME + ".zip"

FILE_NAME_LIST = [
    "/Controller.dll",
    "/CppController.dll",
    "/program.exe",
    "/sounds/4KHz.wav",
    "/sounds/10KHz.wav",
    "/sounds/440Hz.wav",
]

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################


def zip_executable(source, target, env):
    """Create a zip file with the executable and all required libraries."""

    with ZipFile(OUT_PATH, 'w') as zip_object:
        for fileName in FILE_NAME_LIST:
            fullPath = WORKDIR_PATH + fileName
            zip_object.write(fullPath, PIO_ENV_NAME + fileName)

################################################################################
# Main
################################################################################


# Create packed executable as a Post action.
env.AddPostAction(WORKDIR_PATH + "/program.exe",  # type: ignore
                  zip_executable)
