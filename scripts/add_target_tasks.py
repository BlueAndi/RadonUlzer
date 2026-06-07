"""Add customt PlatformIO tasks to select the target."""

# MIT License
#
# Copyright (c) 2022 - 2026 Andreas Merkle (web@blue-andi.de)
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
import re

# pyright: reportUndefinedVariable=false
Import("env") # pylint: disable=undefined-variable

################################################################################
# Variables
################################################################################

PROJECT_PATH = env["PROJECT_DIR"]  # pylint: disable=undefined-variable

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

# pylint: disable=too-many-locals, too-many-branches
def select_target(target_name : str) -> None:
    """Selects the target by modifying the platformio_override.ini file.

    Args:
        target_name: The name of the target to select.
    """
    override_file_path = os.path.join(PROJECT_PATH, "platformio_override.ini")
    target_section = [
        "[hal:Target]\n",
        f"extends = hal:{target_name}\n",
        "extra_scripts =\n",
        f"    {{hal:{target_name}.extra_scripts}}\n"
    ]
    extends_line = f"extends = hal:{target_name}\n"
    target_extra_script_line = f"    {{hal:{target_name}.extra_scripts}}\n"

    if not os.path.exists(override_file_path):
        with open(override_file_path, "w", encoding="utf-8") as override_file:
            override_file.writelines(target_section)
        return

    with open(override_file_path, "r", encoding="utf-8") as override_file:
        lines = override_file.readlines()

    hal_target_start = None
    hal_target_end = len(lines)

    for index, line in enumerate(lines):
        if line.strip() == "[hal:Target]":
            hal_target_start = index
            break

    if hal_target_start is None:
        if len(lines) > 0 and not lines[-1].endswith("\n"):
            lines[-1] += "\n"
        if len(lines) > 0 and lines[-1].strip() != "":
            lines.append("\n")
        lines.extend(target_section)
    else:
        for index in range(hal_target_start + 1, len(lines)):
            stripped_line = lines[index].strip()
            if stripped_line.startswith("[") and stripped_line.endswith("]"):
                hal_target_end = index
                break

        section_lines = lines[hal_target_start:hal_target_end]

        extends_index = None
        extra_scripts_index = None
        target_extra_scripts_index = None

        for index, line in enumerate(section_lines):
            if re.match(r"^\s*extends\s*=", line):
                extends_index = index
            if re.match(r"^\s*extra_scripts\s*=", line):
                extra_scripts_index = index
            if re.search(r"\{\s*hal:[^}]+\.extra_scripts\s*\}", line):
                target_extra_scripts_index = index

        if extends_index is None:
            section_lines.insert(1, extends_line)
        else:
            section_lines[extends_index] = extends_line

        if extra_scripts_index is None:
            section_lines.append("extra_scripts =\n")
            section_lines.append(target_extra_script_line)
        elif target_extra_scripts_index is None:
            section_lines.insert(extra_scripts_index + 1, target_extra_script_line)
        else:
            section_lines[target_extra_scripts_index] = target_extra_script_line

        lines[hal_target_start:hal_target_end] = section_lines

    with open(override_file_path, "w", encoding="utf-8") as override_file:
        override_file.writelines(lines)

################################################################################
# Main
################################################################################

# pylint: disable=undefined-variable
env.AddCustomTarget(
    name="select_zumo32u4_target",
    dependencies=None,
    actions=[
        lambda source, target, env: select_target("Zumo32U4")
    ],
    title="Select Zumo32U4 target",
    description="Selects the Zumo32U4 target by modifying the platformio_override.ini file."
)

# pylint: disable=undefined-variable
env.AddCustomTarget(
    name="select_zumorp2040_target",
    dependencies=None,
    actions=[
        lambda source, target, env: select_target("ZumoRP2040")
    ],
    title="Select ZumoRP2040 target",
    description="Selects the ZumoRP2040 target by modifying the platformio_override.ini file."
)
