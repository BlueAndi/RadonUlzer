"""Copies the sounds to the executable local folder."""

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
import errno
import os

Import("env")  # type: ignore

################################################################################
# Variables
################################################################################

PIO_ENV_NAME = env["PIOENV"]  # type: ignore
DST_PATH = './.pio/build/' + PIO_ENV_NAME + "/sounds"
SOUNDS_FOLDER = "./sounds"

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################


def copy_path_recursive(src, dst):
    """Copy all files from the source path to the destination path.

    Args:
        src (str): Source path
        dst (str): Destination path
    """
    try:
        shutil.copytree(src, dst)
    except OSError as exc:  # python >2.5
        if exc.errno in (errno.ENOTDIR, errno.EINVAL):
            shutil.copy(src, dst)
        else:
            raise

################################################################################
# Main
################################################################################


if os.path.exists(DST_PATH) is False:
    copy_path_recursive(SOUNDS_FOLDER, DST_PATH)
