"""The robot observer retrieves the position and orientation of a robot
    by using a webots supervisor.
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

import math
from typing import Any, List, Tuple

################################################################################
# Variables
################################################################################

################################################################################
# Classes
################################################################################


class RobotObserver():
    """The robot observer provides position and orientation information
        about a robot by using a supervisor.
    """

    def __init__(self, robot_node: Any) -> None:
        self._robot_node = robot_node
        self._ref_position = [0, 0, 0]  # x, y, z
        self._ref_orientation_euler = [0, 0, 0, 0]  # roll, pitch and yaw angle
        self._factor_m_to_mm = 1000

    def _get_position(self) -> List[float]:
        """Get robot position (x, y, z) in m.

        Returns:
            List[float]: Robot position (x, y, z) in m
        """
        position = [0, 0, 0]

        if self._robot_node is not None:
            position = self._robot_node.getPosition()

        return position

    def _get_orientation(self) -> List[float]:
        """Get robot orientation as 3x3 orthogonal rotation matrix.

        Returns:
            List[float]: 3x3 orthogonal rotation matrix
        """
        orientation = [0, 0, 0, 0, 0, 0, 0, 0, 0]

        if self._robot_node is not None:
            orientation = self._robot_node.getOrientation()

        return orientation

    def _get_orientation_euler(self) -> Tuple[float, float, float]:
        """Get euler angles in rad.

        Returns:
            Tuple[float, float, float]: Roll, pitch and yaw angle in rad, range [-PI; PI].
        """
        roll_angle = 0
        pitch_angle = 0
        yaw_angle = 0
        orientation = self._get_orientation()

        if orientation is not None:
            # Extract roll, pitch, and yaw angles from the rotation matrix
            # R23, R23
            roll_angle = math.atan2(orientation[7],
                                    orientation[8])
            # -R13, sqrt(R23^2 + R33^2)
            pitch_angle = math.atan2(-orientation[6],
                                     math.sqrt(orientation[7]**2 + orientation[8]**2))
            # R02, R00
            yaw_angle = math.atan2(orientation[3],
                                   orientation[0])

        return roll_angle, pitch_angle, yaw_angle

    def is_valid(self) -> bool:
        """Returns whether the robot observer is valid.
            If the robot node was not found by name, it will be invalid.

        Returns:
            bool: Robot observer is valid (True) or not (False).
        """
        return self._robot_node is not None

    def set_current_position_as_reference(self) -> None:
        """Set the current robot position as reference.
        """
        self._ref_position = self._get_position()

    def get_rel_position(self) -> List[float]:
        """Get the robot position relative to the reference position.

        Returns:
            List[float]: Relative position (x, y, z) in mm.
        """
        abs_position = self._get_position()

        rel_position = [
            (abs_position[0] - self._ref_position[0]) * self._factor_m_to_mm,
            (abs_position[1] - self._ref_position[1]) * self._factor_m_to_mm,
            (abs_position[2] - self._ref_position[2]) * self._factor_m_to_mm
        ]

        return rel_position

    def set_current_orientation_as_reference(self) -> None:
        """Set the current robot orientation as reference.
        """
        self._ref_orientation_euler = self._get_orientation_euler()

    def get_rel_orientation(self) -> List[float]:
        """Get the robot orientation relative to the reference orientation.
            The angle increases clockwise and decreases counter-clockwise.

        Returns:
            List[float]: Robot orientation (roll, pitch, yaw) in rad, range [-PI; PI].
        """
        abs_orientation_euler = self._get_orientation_euler()

        rel_orientation_euler = [
            abs_orientation_euler[0] - self._ref_orientation_euler[0],
            abs_orientation_euler[1] - self._ref_orientation_euler[1],
            abs_orientation_euler[2] - self._ref_orientation_euler[2]
        ]

        return rel_orientation_euler

    def process_communication(self) -> None:
        """Process communication with the robot.
        """


def rad_to_deg(angle_rad: float) -> float:
    """Convert angle in rad to degree.

    Args:
        angle_rad (float): Angle in rad

    Returns:
        float: Angle in degree
    """
    return angle_rad * 180 / math.pi


def has_position_changed(position: List[float], position_old: List[float]) -> bool:
    """Returns whether the position changed.

    Args:
        position (list[float]): Position 1
        position_old (list[float]): Position 2

    Returns:
        bool: If changed, it will return True otherwise False.
    """
    has_changed = False

    for idx, value in enumerate(position):
        if int(value) != int(position_old[idx]):
            has_changed = True
            break

    return has_changed


def has_orientation_changed(orientation: List[float], orientation_old: List[float]) -> bool:
    """Returns whether the orientation changed.

    Args:
        orientation (list[float]): Orientation 1
        orientation_old (list[float]): Orientation 2

    Returns:
        bool: If changed, it will return True otherwise False.
    """
    has_changed = False

    for idx, value in enumerate(orientation):
        if int(rad_to_deg(value)) != int(rad_to_deg(orientation_old[idx])):
            has_changed = True
            break

    return has_changed

################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################
