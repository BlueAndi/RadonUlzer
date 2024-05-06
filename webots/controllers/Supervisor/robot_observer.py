"""The robot observer retrieves the position and orientation of a robot
    by using a webots supervisor.
"""

import math

class RobotObserver():
    """The robot observer provides position and orientation information
        about a robot by using a supervisor.
    """
    def __init__(self, robot_node):
        self._robot_node = robot_node
        self._ref_position = [0, 0, 0] # x, y, z
        self._ref_orientation_euler = [0, 0, 0, 0] # roll, pitch and yaw angle
        self._factor_m_to_mm = 1000

    def _get_position(self):
        """Get robot position (x, y, z) in m.

        Returns:
            list[float]: Robot position (x, y, z) in m
        """
        position = [0, 0, 0]

        if self._robot_node is not None:
            position = self._robot_node.getPosition()

        return position

    def _get_orientation(self):
        """Get robot orientation as 3x3 orthogonal rotation matrix.

        Returns:
            list[float]: 3x3 orthogonal rotation matrix
        """
        orientation = [0, 0, 0, 0, 0, 0, 0, 0, 0]

        if self._robot_node is not None:
            orientation = self._robot_node.getOrientation()

        return orientation

    def _get_orientation_euler(self):
        """Get euler angles in rad.

        Returns:
            tuple[float, float, float]: Roll, pitch and yaw angle in rad, range [-PI; PI].
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

    def is_valid(self):
        """Returns whether the robot observer is valid.
            If the robot node was not found by name, it will be invalid.

        Returns:
            bool: Robot observer is valid (True) or not (False).
        """
        return self._robot_node is not None

    def set_current_position_as_reference(self):
        """Set the current robot position as reference.
        """
        self._ref_position = self._get_position()

    def get_rel_position(self):
        """Get the robot position relative to the reference position.

        Returns:
            list[float]: Relative position (x, y, z) in mm.
        """
        abs_position = self._get_position()

        rel_position = [
            (abs_position[0] - self._ref_position[0]) * self._factor_m_to_mm,
            (abs_position[1] - self._ref_position[1]) * self._factor_m_to_mm,
            (abs_position[2] - self._ref_position[2]) * self._factor_m_to_mm
        ]

        return rel_position

    def set_current_orientation_as_reference(self):
        """Set the current robot orientation as reference.
        """
        self._ref_orientation_euler = self._get_orientation_euler()

    def get_rel_orientation(self):
        """Get the robot orientation relative to the reference orientation.
            The angle increases clockwise and decreases counter-clockwise.

        Returns:
            list[float]: Robot orientation (roll, pitch, yaw) in rad, range [-PI; PI].
        """
        abs_orientation_euler = self._get_orientation_euler()

        rel_orientation_euler = [
            abs_orientation_euler[0] - self._ref_orientation_euler[0],
            abs_orientation_euler[1] - self._ref_orientation_euler[1],
            abs_orientation_euler[2] - self._ref_orientation_euler[2]
        ]

        return rel_orientation_euler

    def process_communication(self):
        """Process communication with the robot.
        """


def rad_to_deg(angle_rad):
    """Convert angle in rad to degree.

    Args:
        angle_rad (float): Angle in rad

    Returns:
        float: Angle in degree
    """
    return angle_rad * 180 / math.pi

def has_position_changed(position, position_old):
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

def has_orientation_changed(orientation, orientation_old):
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
