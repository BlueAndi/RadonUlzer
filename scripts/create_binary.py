"""Create a binary file from the .hex file. 
Required for flashing the firmware through the DCS."""

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

# pyright: reportUndefinedVariable=false
Import("env")  # pylint: disable=undefined-variable

################################################################################
# Variables
################################################################################

BUILD_DIR = env["BUILD_DIR"]  # pylint: disable=undefined-variable
PROGRAM_PATH = os.path.join(BUILD_DIR, "firmware")
INPUT_FILE = PROGRAM_PATH + ".hex"
OUTPUT_FILE = PROGRAM_PATH + ".bin"

# Command to convert .hex into .bin.
# Use the objcopy tool, which is part of the toolchain.
CMD = "${OBJCOPY} -I ihex \"" + INPUT_FILE +\
    "\" -O binary \"" + OUTPUT_FILE + "\""

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################


def create_binary(source, target, env) -> None:
    """Create a binary file from the .hex file.
    Arguments:
        source: Output binary of build process.
        target: File that triggered this callback. INPUT_FILE in this case.
        env: SCons build environment

    Return:
        None. Exits on fail.
    """
    # pylint: disable=unused-argument

    # Convert .hex into .bin using objcopy.
    print("Creating binary file...")
    print(CMD)

    if 0 != env.Execute(CMD):
        print("Command failed!")
        Exit(1)  # pylint: disable=undefined-variable


################################################################################
# Main
################################################################################

# Always build .hex file. Assume it is always out-of-date
# pylint: disable=undefined-variable
env.AlwaysBuild(INPUT_FILE)  # type: ignore

# Register the callback to the creation of the .hex file
# pylint: disable=undefined-variable
env.AddPostAction(INPUT_FILE, create_binary)  # type: ignore
