""" Main programm entry point"""

# MIT License
#
# Copyright (c) 2023 Gabryel Reyes <gabryelrdiaz@gmail.com>
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
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#


################################################################################
# Imports
################################################################################

import sys
import time
from struct import Struct
import keyboard # pylint: disable=import-error
import numpy as np
from yap import YAP
from socket_client import SocketClient

################################################################################
# Variables
################################################################################

g_socket = SocketClient("localhost", 65432)
yap_server = YAP(10, g_socket)
START_TIME = round(time.time()*1000)

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################


def get_milliseconds() -> int:
    """ Get current system milliseconds """
    return round(time.time()*1000)


def millis() -> int:
    """ Get current program milliseconds """
    current_time = get_milliseconds()
    return current_time - START_TIME


def callback_line_sensors(payload: bytearray) -> None:
    """ Callback of LINE_SENS Channel """
    unpacker = Struct(">HHHHH")
    data = unpacker.unpack_from(payload)
    print(np.array(data, dtype=np.int16))

def callback_remote_response(payload:bytearray) -> None:
    """ Callback of REMOTE_CMD Channel """
    if payload == 0:
        print("Command OK")
    elif payload == 1:
        print("Command Pending")
    elif payload == 2:
        print("Command Error")

def send_motor_setpoints(set_point_left: int, set_point_right: int):
    """
    Send Motor Setpoints
    """

    packer = Struct(">H")
    payload = bytearray()
    payload.extend(packer.pack(set_point_left))
    payload.extend(packer.pack(set_point_right))

    if len(payload) == 4:
        yap_server.send_data("MOT_SPEEDS", payload)

def send_command(command: str) -> None:
    """Send command to RadonUlzer"""

    payload = bytearray()

    if command == "line_calib":
        payload.append(0x01)
    elif command == "motor_calib":
        payload.append(0x02)
    elif command == "enable_drive":
        payload.append(0x03)

    yap_server.send_data("REMOTE_CMD", payload)


def main():
    """The program entry point function.adadadadada
    Returns:
        int: System exit status
    """
    print("Starting System.")
    last_time = 0

    try:
        g_socket.connect_to_server()
    except Exception as err:  # pylint: disable=broad-exception-caught
        print(err)
        return

    yap_server.create_channel("MOT_SPEEDS", 4)
    yap_server.create_channel("REMOTE_CMD", 1)
    yap_server.subscribe_to_channel("REMOTE_RSP", callback_remote_response)
    yap_server.subscribe_to_channel("LINE_SENS", callback_line_sensors)

    keyboard.on_press_key("w", lambda e: send_motor_setpoints(0x8000, 0x8000))
    keyboard.on_press_key("s", lambda e: send_motor_setpoints(0x7FFF, 0x7FFF))
    keyboard.on_press_key("a", lambda e: send_motor_setpoints(0x7FFF, 0x8000))
    keyboard.on_press_key("d", lambda e: send_motor_setpoints(0x8000, 0x7FFF))
    keyboard.on_release_key("w", lambda e: send_motor_setpoints(0x0000, 0x0000))
    keyboard.on_release_key("a", lambda e: send_motor_setpoints(0x0000, 0x0000))
    keyboard.on_release_key("s", lambda e: send_motor_setpoints(0x0000, 0x0000))
    keyboard.on_release_key("d", lambda e: send_motor_setpoints(0x0000, 0x0000))
    keyboard.on_press_key("l", lambda e: send_command("line_calib"))
    keyboard.on_press_key("m", lambda e: send_command("motor_calib"))
    keyboard.on_press_key("e", lambda e: send_command("enable_drive"))

    while True:
        if (millis() - last_time) >= 5:
            last_time = millis()
            yap_server.process(millis())

################################################################################
# Main
################################################################################


if __name__ == "__main__":
    sys.exit(main())
