"""Pack the executable and its dependencies into a ZIP file."""

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
import platform
import sys
from zipfile import ZipFile

Import("env") # pylint: disable=undefined-variable

################################################################################
# Variables
################################################################################

OS_PLATFORM_TYPE_WIN = "Windows"
OS_PLATFORM_TYPE_LINUX = "Linux"
OS_PLATFORM_TYPE_MACOS = "Darwin"
OS_PLATFORM_TYPE = platform.system()

PIO_ENV_NAME = env["PIOENV"] # pylint: disable=undefined-variable
BUILD_DIR = env["PROJECT_BUILD_DIR"] + "/" + PIO_ENV_NAME # pylint: disable=undefined-variable
OUT_PATH = BUILD_DIR + "/" + PIO_ENV_NAME + ".zip"

FILE_NAME_LIST = []

if OS_PLATFORM_TYPE == OS_PLATFORM_TYPE_WIN:

    FILE_NAME_LIST = [
        "/Controller.dll",
        "/CppController.dll",
        "/program.exe",
        "/sounds/4KHz.wav",
        "/sounds/10KHz.wav",
        "/sounds/440Hz.wav",
    ]

elif OS_PLATFORM_TYPE == OS_PLATFORM_TYPE_MACOS:

    FILE_NAME_LIST = [
        "/Controller.dylib",
        "/CppController.dylib",
        "/program",
        "/sounds/4KHz.wav",
        "/sounds/10KHz.wav",
        "/sounds/440Hz.wav",
    ]

elif OS_PLATFORM_TYPE == OS_PLATFORM_TYPE_LINUX:

    FILE_NAME_LIST = [
        "/Controller.so",
        "/CppController.so",
        "/program",
        "/sounds/4KHz.wav",
        "/sounds/10KHz.wav",
        "/sounds/440Hz.wav",
    ]

else:
    print(f"OS type {OS_PLATFORM_TYPE} not supported.")
    sys.exit(1)

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

def zip_executable(source, target, env): # pylint: disable=unused-argument
    """Create a zip file with the executable and all required libraries.

    Args:
        source (Any): Source (not used)
        target (Any): Target (not used)
        env (Any): Environment (not used)
    """

    with ZipFile(OUT_PATH, 'w') as zip_object:
        for file_name in FILE_NAME_LIST:
            full_path = BUILD_DIR + file_name
            zip_object.write(full_path, PIO_ENV_NAME + file_name)

    print("Packed executable found in: " + OUT_PATH)

################################################################################
# Main
################################################################################

# Create packed executable as a Post action.
env.AddPostAction(BUILD_DIR + "/program.exe", zip_executable) # pylint: disable=undefined-variable
