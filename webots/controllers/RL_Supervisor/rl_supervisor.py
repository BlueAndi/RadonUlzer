"""
Supervisor controller.

Details: https://github.com/cyberbotics/webots/blob/master/docs/guide/supervisor-programming.md
"""

# MIT License
#
# Copyright (c) 2023 - 2024 Andreas Merkle <web@blue-andi.de>
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
import struct
from controller import Supervisor  # pylint: disable=import-error
from serial_webots import SerialWebots
from SerialMuxProt import Server

################################################################################
# Variables
################################################################################


# Constants
ROBOT_NAME = "ROBOT"
SUPERVISOR_RX_NAME = "serialComRx"
SUPERVISOR_TX_NAME = "serialComTx"

COMMAND_CHANNEL_NAME = "CMD"
CMD_DLC = 1

SPEED_SET_CHANNEL_NAME = "SPEED_SET"
SPEED_SET_DLC = 4

LINE_SENSOR_CHANNEL_NAME = "LINE_SENS"
LINE_SENSOR_ON_TRACK_MIN_VALUE = 200

STATUS_CHANNEL_NAME = "STATUS"
STATUS_CHANNEL_ERROR_VAL = 1

MODE_CHANNEL_NAME = "MODE"
CMD_ID_SET_READY_STATE = 1
POSITION_DATA = [-0.24713614078815466, -0.04863962992854465, 0.013994298332013683]
ORIENTATION_DATA = [
    -1.0564747468923541e-06,
    8.746699709178704e-07,
    0.9999999999990595,
    1.5880805820884731,
]


################################################################################
# Classes
################################################################################


class RobotController:
    """Class for data flow control logic"""

    def __init__(self, robot_node, smp_server, tick_size):
        self.__robot_node = robot_node
        self.__smp_server = smp_server
        self.__tick_size = tick_size
        self.__no_line_detection_count = 0
        self.__timestamp = 0  # Elapsed time since reset [ms]
        self.is_sequence_complete = False

    def reinitialize(self):
        """Re-initialization of position and orientation of The ROBOT."""
        trans_field = self.__robot_node.getField("translation")
        rot_field = self.__robot_node.getField("rotation")
        initial_position = POSITION_DATA
        initial_orientation = ORIENTATION_DATA
        trans_field.setSFVec3f(initial_position)
        rot_field.setSFRotation(initial_orientation)

    def callback_status(self, payload: bytearray) -> int:
        """Callback Status Channel."""

        # perform action on robot status feedback
        if payload[0] == STATUS_CHANNEL_ERROR_VAL:
            print("robot has reached error-state (max. lap time passed in robot)")

            # stop robot motors
            self.__smp_server.send_data(SPEED_SET_CHANNEL_NAME, struct.pack("2H", 0, 0))

            # switch robot to ready state
            self.__smp_server.send_data(
                COMMAND_CHANNEL_NAME, struct.pack("B", CMD_ID_SET_READY_STATE)
            )

            # perform robot state reset
            self.is_sequence_complete = True

    def callback_line_sensors(self, payload: bytearray) -> None:
        """Callback LINE_SENS Channel."""
        sensor_data = struct.unpack("5H", payload)

        for idx in range(5):
            if idx == 4:
                print(f"Sensor[{idx}] = {sensor_data[idx]}")
            else:
                print(f"Sensor[{idx}] = {sensor_data[idx]}, ", end="")

        # determine lost line condition
        if all(value == 0 for value in sensor_data):
            self.__no_line_detection_count += 1
        else:
            self.__no_line_detection_count = 0

        # detect start/stop line
        is_start_stop = all(
            value >= LINE_SENSOR_ON_TRACK_MIN_VALUE for value in sensor_data
        )

        # sequence stop criterion debounce no line detection and
        if (self.__no_line_detection_count >= 20) or is_start_stop:
            # Stop motors, maximum NO Line Detection Counter reached
            self.__smp_server.send_data(SPEED_SET_CHANNEL_NAME, struct.pack("2H", 0, 0))

            # SENDING A COMMAND ID SET READY STATUS
            self.__smp_server.send_data(
                COMMAND_CHANNEL_NAME, struct.pack("B", CMD_ID_SET_READY_STATE)
            )
            self.is_sequence_complete = True

        else:
            # [PLACEHOLDER] replace with line sensor data receiver

            # Set left and right motors speed to 1000
            self.__smp_server.send_data(
                SPEED_SET_CHANNEL_NAME, struct.pack("2H", 1000, 1000)
            )

    def callback_mode(self, payload: bytearray) -> None:
        """Callback MODE Channel."""
        driving_mode = payload[0]

        if driving_mode:
            print("Driving Mode Selected.")
        else:
            print("Train Mode Selected.")

    def process(self):
        """function performing controller step"""
        self.__timestamp += self.__tick_size

        # process new data (callbacks will be executed)
        self.__smp_server.process(self.__timestamp)


################################################################################
# Functions
################################################################################


# pylint: disable=duplicate-code
def main_loop():
    """Main loop:
        - Perform simulation steps until Webots is stopping the controller.

    Returns:
        number: Status
    """
    status = 0

    supervisor = Supervisor()
    timestep = int(supervisor.getBasicTimeStep())

    # get serial receiver from supervisor
    supervisor_com_rx = supervisor.getDevice(SUPERVISOR_RX_NAME)
    if supervisor_com_rx is None:
        print(f"ERROR: {SUPERVISOR_RX_NAME} not found.")
        status = -1
    else:
        supervisor_com_rx.enable(timestep)
        supervisor_com_rx.setChannel(2)

    # get serial emitter from supervisor
    supervisor_com_tx = supervisor.getDevice(SUPERVISOR_TX_NAME)
    if supervisor_com_tx is None:
        print(f"ERROR: {SUPERVISOR_TX_NAME} not found.")
        status = -1
    else:
        # supervisor_com_tx.enable(timestep)
        supervisor_com_tx.setChannel(1)

    # get robot defition
    robot_node = supervisor.getFromDef(ROBOT_NAME)
    if robot_node is None:
        print(f"ERROR: {ROBOT_NAME} not found.")
        status = -1

    # connect webots serial nodes to SerMuxProt
    s_client = SerialWebots(supervisor_com_tx, supervisor_com_rx)
    smp_server = Server(10, s_client)

    sermux_channel_speed_set = smp_server.create_channel(
        SPEED_SET_CHANNEL_NAME, SPEED_SET_DLC
    )
    sermux_channel_cmd_id = smp_server.create_channel(COMMAND_CHANNEL_NAME, CMD_DLC)

    if sermux_channel_speed_set == 0:
        print("ERROR: channel SPEED_SET not created.")
        status = -1

    if sermux_channel_cmd_id == 0:
        print("ERROR: channel CMD not created.")
        status = -1

    # create instance of robot logic class
    controller = RobotController(robot_node, smp_server, timestep)

    smp_server.subscribe_to_channel(STATUS_CHANNEL_NAME, controller.callback_status)

    smp_server.subscribe_to_channel(
        LINE_SENSOR_CHANNEL_NAME, controller.callback_line_sensors
    )

    smp_server.subscribe_to_channel(MODE_CHANNEL_NAME, controller.callback_mode)

    # setup successful
    if status != -1:
        # simulation loop
        while supervisor.step(timestep) != -1:
            controller.process()

            # stopping condition for sequence was reached
            if controller.is_sequence_complete is True:
                controller.reinitialize()
                controller.is_sequence_complete = False

    return status


sys.exit(main_loop())


################################################################################
# Main
################################################################################
