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
import os
from controller import Supervisor  # pylint: disable=import-error
from serial_webots import SerialWebots
from SerialMuxProt import Server
from agent import Agent

################################################################################
# Variables
################################################################################
# pylint: disable=duplicate-code

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

MIN_NUMBER_OF_STEPS = 400
SENSOR_ID_MOST_LEFT = 0
SENSOR_ID_MOST_RIGHT = 4

IDLE = "IDLE_STATE"
READY = "READY_STATE"
TRAINING = "TRAINING_STATE"

# Path of saved models
PATH = "models/"

################################################################################
# Classes
################################################################################


class RobotController:
    """Class for data flow control logic."""

    def __init__(self, smp_server, tick_size, agent):
        self.__smp_server = smp_server
        self.__agent = agent
        self.__tick_size = tick_size
        self.__no_line_detection_count = 0
        self.__timestamp = 0  # Elapsed time since reset [ms]
        self.last_sensor_data = None
        self.steps = 0

    def callback_status(self, payload: bytearray) -> None:
        """Callback Status Channel."""

        # perform action on robot status feedback
        if payload[0] == STATUS_CHANNEL_ERROR_VAL:
            print("robot has reached error-state (max. lap time passed in robot)")
            self.__agent.done = True

    def callback_line_sensors(self, payload: bytearray) -> None:
        """Callback LINE_SENS Channel."""
        sensor_data = struct.unpack("5H", payload)
        self.steps += 1

        is_start_stop_line_detected = False
        # Determine lost line condition
        if all(value == 0 for value in sensor_data):
            self.__no_line_detection_count += 1
        else:
            self.__no_line_detection_count = 0

        # Detect start/stop line
        if ((sensor_data[SENSOR_ID_MOST_LEFT] >= LINE_SENSOR_ON_TRACK_MIN_VALUE) and
            (sensor_data[SENSOR_ID_MOST_RIGHT] >= LINE_SENSOR_ON_TRACK_MIN_VALUE)):
            is_start_stop_line_detected = True

        # Detect Start/Stop Line before Finish Trajectories
        if (is_start_stop_line_detected is True) and (self.steps < MIN_NUMBER_OF_STEPS):
            sensor_data = list(sensor_data)
            sensor_data[SENSOR_ID_MOST_LEFT]  = 0
            sensor_data[SENSOR_ID_MOST_RIGHT] = 0
            is_start_stop_line_detected       = False

        # sequence stop criterion debounce no line detection and start/stop line detected
        if ((self.__no_line_detection_count >= 30) or ((is_start_stop_line_detected is True)
                                                and  (self.steps >= MIN_NUMBER_OF_STEPS))):
            self.__agent.done = True
            self.__no_line_detection_count = 0
            self.steps = 0

        # The sequence of states and actions is stored in memory for the training phase.
        if self.__agent.train_mode:

            # receive a -1 punishment if the robot leaves the line
            if self.__no_line_detection_count > 0:
                reward = -1
            else:
                reward = self.__agent.determine_reward(sensor_data)

            # Start storage The data after the second received sensor data
            if self.last_sensor_data is not None:
                self.__agent.store_transition(
                    self.last_sensor_data,
                    self.__agent.action,
                    self.__agent.adjusted_log_prob,
                    self.__agent.value,
                    reward,
                    self.__agent.done,
                )
            self.last_sensor_data = sensor_data

        # Sends the motor speeds to the robot.
        if self.__agent.done is False and self.__agent.state == READY:
            self.__agent.send_motor_speeds(sensor_data)

    def callback_mode(self, payload: bytearray) -> None:
        """Callback MODE Channel."""
        driving_mode = payload[0]

        if driving_mode:
            self.__agent.set_drive_mode()

        else:
            self.__agent.set_train_mode()

    def load_models(self, path) -> None:
        """Load Model if exist"""
        if os.path.exists(path):
            self.__agent.load_models()
        else:
            print("No model available")

    def retry_unsent_data(self, unsent_data: list) -> bool:
        """Resent any unsent Data"""
        retry_succesful = True

        # Resent the unsent Data.
        for data in unsent_data[:]:
            if self.__smp_server.send_data(data[0], data[1]) is True:
                unsent_data.remove(data)
            else:
                retry_succesful = False

        return retry_succesful

    def process(self):
        """function performing controller step"""
        self.__timestamp += self.__tick_size

        # process new data (callbacks will be executed)
        self.__smp_server.process(self.__timestamp)

    def manage_agent_cycle(self,robot_node):
        """The function controls agent behavior"""
        if self.__agent.state == READY:
            self.__agent.update(robot_node)

        # Start the training
        elif self.__agent.state == TRAINING:
            self.last_sensor_data = None
            self.__agent.perform_training()

            # save model
            if (self.__agent.num_episodes > 1) and (self.__agent.num_episodes % 50 == 0):
                self.__agent.save_models()


################################################################################
# Functions
################################################################################


# pylint: disable=duplicate-code
# pylint: disable=too-many-statements
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
    # create instance of intelligence Agent
    agent = Agent(smp_server)

    # create instance of robot logic class
    controller = RobotController(smp_server, timestep, agent)

    smp_server.subscribe_to_channel(STATUS_CHANNEL_NAME, controller.callback_status)

    smp_server.subscribe_to_channel(
        LINE_SENSOR_CHANNEL_NAME, controller.callback_line_sensors
    )

    smp_server.subscribe_to_channel(MODE_CHANNEL_NAME, controller.callback_mode)

    # setup successful
    if status != -1:

        controller.load_models(PATH)

        # simulation loop
        while supervisor.step(timestep) != -1:
            controller.process()

            controller.manage_agent_cycle(robot_node)

            # Resent any unsent Data
            if agent.unsent_data:

                # Stop The Simulation. Handle unsent Data
                supervisor.simulationSetMode(Supervisor.SIMULATION_MODE_PAUSE)

                # Set simulation mode to real time when unsent data is resent
                if controller.retry_unsent_data(agent.unsent_data) is True:
                    supervisor.simulationSetMode(Supervisor.SIMULATION_MODE_REAL_TIME)

                # Reset The Simulation
                else:
                    supervisor.simulationReset()

    return status


sys.exit(main_loop())


################################################################################
# Main
################################################################################
