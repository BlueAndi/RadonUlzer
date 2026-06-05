"""Supervisor controller.
    Details: https://github.com/cyberbotics/webots/blob/master/docs/guide/supervisor-programming.md
"""

# MIT License
#
# Copyright (c) 2019 - 2025 Andreas Merkle (web@blue-andi.de)
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

# pyright: reportMissingImports=false

import math
import sys
from dataclasses import dataclass
from my_supervisor import MySupervisor
from robot_observer import RobotObserver, has_position_changed, has_orientation_changed

################################################################################
# Variables
################################################################################

# The PROTO DEF name must be given!
ROBOT_NAME = "ROBOT"

# The supervisor receiver name.
SUPERVISOR_RX_NAME = "supervisorComRx"

# The supervisor sender name.
SUPERVISOR_TX_NAME = "supervisorComTx"

# The supervisor communication channel IDs.
SUPERVISOR_RX_CHANNEL_ID = 2
SUPERVISOR_TX_CHANNEL_ID = 1

################################################################################
# Classes
################################################################################


@dataclass
class OdometryData():
    """Odometry data container.
    """
    x: int = 0
    y: int = 0
    yaw_angle: float = 0

    def reset(self) -> None:
        """Reset odometry data.
        """
        self.x = 0
        self.y = 0
        self.yaw_angle = 0

################################################################################
# Functions
################################################################################


def rad_to_deg(angle_rad: float) -> float:
    """Convert angle in rad to degree.

    Args:
        angle_rad (float): Angle in rad

    Returns:
        float: Angle in degree
    """
    return angle_rad * 180 / math.pi


def convert_angle_to_2pi(angle: float) -> float:
    """Convert angle from range [-PI; PI] in rad to [0; 2PI].

    Args:
        angle (float): Angle in rad, range [-PI; PI].

    Returns:
        float: Angle in rad, range [0; 2PI]
    """
    if angle < 0:
        angle += 2 * math.pi

    return angle


def convert_webots_angle_to_ru_angle(webots_angle: float) -> float:
    """Convert Webots angle to RadonUlzer angle.

    Args:
        webots_angle (float): Webots angle in rad, range [-PI; PI].

    Returns:
        float: Angle in rad, range [-2PI; 2PI]
    """
    # Radon Ulzer
    #   Angle [-2PI; 2PI]
    #   North are 90° (PI/2)
    #
    # Webots angle [-PI; PI]
    webots_angle += math.pi / 2

    webots_angle_2pi = convert_angle_to_2pi(webots_angle)

    # TO DO Handling the negative range.

    return webots_angle_2pi


def main_loop():
    """Main loop:
        - Perform simulation steps until Webots is stopping the controller-

    Returns:
        number: Status
    """
    status = 0

    print("*** Supervisor ***")

    # Create the Supervisor instance.
    supervisor = MySupervisor(SUPERVISOR_RX_NAME, SUPERVISOR_TX_NAME,
                              SUPERVISOR_RX_CHANNEL_ID, SUPERVISOR_TX_CHANNEL_ID)

    # Get robot node which to observe.
    robot_node = supervisor.getFromDef(ROBOT_NAME)

    if robot_node is None:

        print(f"Supervisor: Robot DEF {ROBOT_NAME} not found.")
        status = -1

    else:
        robot_observer = RobotObserver(robot_node)

        # Set current position and orientation in the map as reference.
        robot_observer.set_current_position_as_reference()
        robot_observer.set_current_orientation_as_reference()

        robot_position_old = robot_observer.get_rel_position()
        robot_orientation_old = robot_observer.get_rel_orientation()
        robot_odometry = OdometryData()

        while supervisor.step() != -1:
            rx_data = supervisor.receive()

            if rx_data is None:
                pass

            else:
                command = rx_data.split(',')

                # Reset robot position and orientation?
                if command[0] == "RST":
                    print("Supervisor: RST")
                    robot_observer.set_current_position_as_reference()
                    robot_observer.set_current_orientation_as_reference()
                    robot_odometry.reset()

                # Robot odometry data received?
                elif command[0] == "ODO":
                    robot_odometry.x = int(command[1])  # [mm]
                    robot_odometry.y = int(command[2])  # [mm]
                    robot_odometry.yaw_angle = float(
                        command[3]) / 1000.0  # [rad]

                # Unknown command.
                else:
                    print(f"Supervisor: Unknown command: {command[0]}")

            robot_position = robot_observer.get_rel_position()
            robot_orientation = robot_observer.get_rel_orientation()

            any_change = has_position_changed(
                robot_position, robot_position_old)

            if any_change is False:
                any_change = has_orientation_changed(
                    robot_orientation, robot_orientation_old)

            if any_change is True:

                robot_yaw_angle = robot_orientation[2]
                yaw_ru_angle = convert_webots_angle_to_ru_angle(
                    robot_yaw_angle)

                print("Supervisor: "
                      f"{int(robot_position[0])}, "
                      f"{int(robot_position[1])}, "
                      f"{int(rad_to_deg(yaw_ru_angle))} /"
                      f" odo: {robot_odometry.x}, "
                      f"{robot_odometry.y}, "
                      f"{int(rad_to_deg(robot_odometry.yaw_angle))}")

                robot_position_old = robot_position
                robot_orientation_old = robot_orientation

    return status

################################################################################
# Main
################################################################################

if __name__ == "__main__":
    sys.exit(main_loop())
