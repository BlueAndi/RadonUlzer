"""Webots library creation"""

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
import sys
import shutil
import errno
import os
import json
import platform

################################################################################
# Variables
################################################################################

src_path_include_c      = os.getenv('WEBOTS_HOME') + '/include/controller/c'
src_path_include_cpp    = os.getenv('WEBOTS_HOME') + '/include/controller/cpp'
src_path_source         = os.getenv('WEBOTS_HOME') + '/src/controller/cpp'
src_path_lib            = os.getenv('WEBOTS_HOME') + '/lib/controller/libController'

OS_PLATFORM_TYPE_WIN = "Windows"
OS_PLATFORM_TYPE_LINUX = "Linux"
OS_PLATFORM_TYPE_MACOS = "Darwin"
OS_PLATFORM_TYPE = platform.system()

#  Add correct file extension for the platform.
if OS_PLATFORM_TYPE == OS_PLATFORM_TYPE_WIN:

    src_path_lib += '.a'

elif OS_PLATFORM_TYPE == OS_PLATFORM_TYPE_MACOS:

    src_path_lib += '.dylib'

elif OS_PLATFORM_TYPE == OS_PLATFORM_TYPE_LINUX:

    src_path_lib += '.so'

else:
    print(f"OS type {OS_PLATFORM_TYPE} not supported.")
    sys.exit(1)

DST_PATH = './lib/Webots'

library_json = {
    "name": "Webots",
    "version": "0.1.0",
    "platforms": ["native"]
}

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
    except OSError as exc: # python >2.5
        if exc.errno in (errno.ENOTDIR, errno.EINVAL):
            shutil.copy(src, dst)
        else: raise

def copy_single_file(src, dst):
    """Copy a single source file to the destination path.

    Args:
        src (str): Source file
        dst (str): Destination path
    """
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    shutil.copy(src, dst)

################################################################################
# Main
################################################################################

if os.path.exists(DST_PATH) is False:
    print(f'Create WEBOTS library to {DST_PATH}.')

    copy_path_recursive(src_path_include_c, DST_PATH + '/include/c/')
    copy_path_recursive(src_path_include_cpp, DST_PATH + '/include/cpp/')
    copy_path_recursive(src_path_source, DST_PATH + '/src/')
    copy_single_file(src_path_lib, DST_PATH + '/lib/')

    with open(DST_PATH + '/library.json', 'w', encoding="utf-8") as fd:
        json.dump(library_json, fd, ensure_ascii=False, indent=4)
