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
NO_LINE_TERMINATION_STEPS = 8

# Supervisor PROTO device names (supervisorComRx / supervisorComTx).
# The rl_supervisor is launched via webots_launcher_zumo_com_system so the robot
# runs with ZumoComSystem enabled (-c flag).  SerialMuxProt travels on the
# robot's normal serial (channels 3/4); the supervisor serial (channels 1/2)
# carries ODO text that is irrelevant for RL training.
SUPERVISOR_RX_NAME = "supervisorComRx"
SUPERVISOR_TX_NAME = "supervisorComTx"

# Channel IDs matching webots_robot_serial_rx/tx_channel in platformio.ini [hal:Sim]
# (robot normal serial RX=3, TX=4).
SUPERVISOR_RX_CHANNEL = 4   # supervisor receives from robot normal-serial TX
SUPERVISOR_TX_CHANNEL = 3   # supervisor sends to robot normal-serial RX

COMMAND_CHANNEL_NAME = "CMD"
# sizeof(Command) in APPRemoteControl SerialMuxChannels.h:
# 1 byte CmdId + union { 3x int32 } = 13 bytes
CMD_DLC = 13

MOTOR_SPEED_CHANNEL_NAME = "MOTOR_SET"
MOTOR_SPEED_DLC = 8  # MotorSpeed: 2x int32 (left, right) in mm/s

LINE_SENSOR_CHANNEL_NAME = "LINE_SENS"
LINE_SENSOR_ON_TRACK_MIN_VALUE = 200

STATUS_CHANNEL_NAME = "STATUS"
STATUS_CHANNEL_ERROR_VAL = 1

# Command IDs matching SMPChannelPayload::CmdId in SerialMuxChannels.h
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
            # Status errors also end the current episode, but bypass the
            # line-sensor termination path where episode steps are recorded.
            self.__agent.complete_episode(self.steps)
            self.steps = 0
            self.__no_line_detection_count = 0

    def callback_line_sensors(self, payload: bytearray) -> None:
        """Callback LINE_SENS Channel."""
        sensor_data = struct.unpack("5H", payload)

        # First LINE_SENS proves SMP is synced and the robot is in DrivingState.
        # Webots DistanceSensor values are already in the 0-1000 range without
        # a separate calibration step, so enter training mode immediately.
        if self.__agent.state == IDLE:
            self.__agent.done = False
            self.__agent.set_train_mode()
            return

        self.steps += 1

        is_start_stop_line_detected = False
        is_ignored_start_stop_line = False

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
            is_ignored_start_stop_line = True
            sensor_data = list(sensor_data)
            sensor_data[SENSOR_ID_MOST_LEFT] = 0
            sensor_data[SENSOR_ID_MOST_RIGHT] = 0
            is_start_stop_line_detected = False

        is_first_no_line_sample = self.__no_line_detection_count == 1
        is_no_line_sample = self.__no_line_detection_count > 0
        is_no_line_terminal = self.__no_line_detection_count >= NO_LINE_TERMINATION_STEPS

        # sequence stop criterion: debounce no-line and start/stop-line detection
        if (is_no_line_terminal
                or ((is_start_stop_line_detected is True)
                    and (self.steps >= MIN_NUMBER_OF_STEPS))):
            self.__agent.done = True
            # Record episode-level diagnostics before resetting the step counter.
            self.__agent.complete_episode(self.steps)
            self.steps = 0
            self.__no_line_detection_count = 0

        # The sequence of states and actions is stored in memory for the training phase.
        if self.__agent.train_mode:

            # Penalize only the action that first caused the robot to lose the
            # line. Further no-line samples carry no additional information.
            if is_first_no_line_sample:
                reward = -1
            elif is_no_line_sample:
                reward = 0
            # Do not reward driving in circles over the start line.
            elif is_ignored_start_stop_line is True:
                reward = 0
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

    def load_models(self, path) -> None:
        """Load Model if exist"""
        actor_path = os.path.join(path, "actor.weights.h5")
        critic_path = os.path.join(path, "critic.weights.h5")

        if os.path.exists(actor_path) and os.path.exists(critic_path):
            self.__agent.load_models()
        else:
            print("No complete model checkpoint available. Starting fresh training.")

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

    def manage_agent_cycle(self, robot_node):
        """The function controls agent behavior"""
        if self.__agent.state == READY:
            self.__agent.update(robot_node)
            # Clear stale sensor context whenever the robot is teleported so
            # the first LINE_SENS of the new episode is treated as an initial
            # observation rather than a transition from the previous episode.
            if self.__agent.reinitialized:
                self.last_sensor_data = None
                self.__agent.reinitialized = False

        # Start the training
        elif self.__agent.state == TRAINING:
            self.last_sensor_data = None
            self.__agent.perform_training()

            # save model
            if (self.__agent.num_training_updates > 1) and (self.__agent.num_training_updates % 50 == 0):
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
        supervisor_com_rx.setChannel(SUPERVISOR_RX_CHANNEL)

    # get serial emitter from supervisor
    supervisor_com_tx = supervisor.getDevice(SUPERVISOR_TX_NAME)
    if supervisor_com_tx is None:
        print(f"ERROR: {SUPERVISOR_TX_NAME} not found.")
        status = -1
    else:
        supervisor_com_tx.setChannel(SUPERVISOR_TX_CHANNEL)

    # get robot definition
    robot_node = supervisor.getFromDef(ROBOT_NAME)
    if robot_node is None:
        print(f"ERROR: {ROBOT_NAME} not found.")
        status = -1

    # connect webots serial nodes to SerialMuxProt
    s_client = SerialWebots(supervisor_com_tx, supervisor_com_rx)
    smp_server = Server(10, s_client)

    sermux_channel_motor_speed = smp_server.create_channel(
        MOTOR_SPEED_CHANNEL_NAME, MOTOR_SPEED_DLC
    )
    sermux_channel_cmd = smp_server.create_channel(
        COMMAND_CHANNEL_NAME, CMD_DLC)

    if sermux_channel_motor_speed == 0:
        print("ERROR: channel MOTOR_SET not created.")
        status = -1

    if sermux_channel_cmd == 0:
        print("ERROR: channel CMD not created.")
        status = -1

    # create instance of intelligence Agent
    agent = Agent(smp_server)

    # create instance of robot logic class
    controller = RobotController(smp_server, timestep, agent)

    smp_server.subscribe_to_channel(
        STATUS_CHANNEL_NAME, controller.callback_status)

    smp_server.subscribe_to_channel(
        LINE_SENSOR_CHANNEL_NAME, controller.callback_line_sensors
    )

    # setup successful
    if status != -1:

        controller.load_models(PATH)

        supervisor.simulationSetMode(Supervisor.SIMULATION_MODE_FAST)

        # Training mode is entered on the first LINE_SENS callback (SMP synced,
        # robot already in DrivingState from startup).

        # simulation loop
        while supervisor.step(timestep) != -1:
            controller.process()

            controller.manage_agent_cycle(robot_node)

            # Resent any unsent Data
            if agent.unsent_data:

                # Stop The Simulation. Handle unsent Data
                supervisor.simulationSetMode(Supervisor.SIMULATION_MODE_PAUSE)

                # Restore fast mode when unsent data is resent
                if controller.retry_unsent_data(agent.unsent_data) is True:
                    supervisor.simulationSetMode(Supervisor.SIMULATION_MODE_FAST)

                # Reset The Simulation
                else:
                    supervisor.simulationReset()

    return status


sys.exit(main_loop())


################################################################################
# Main
################################################################################
