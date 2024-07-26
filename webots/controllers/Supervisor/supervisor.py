"""Supervisor controller.
    Details: https://github.com/cyberbotics/webots/blob/master/docs/guide/supervisor-programming.md
"""

import math
import sys
from dataclasses import dataclass
from controller import Supervisor  # pylint: disable=import-error
from robot_observer import RobotObserver, has_position_changed, has_orientation_changed

# The PROTO DEF name must be given!
ROBOT_NAME = "ROBOT"

# The supervisor receiver name.
SUPERVISOR_RX_NAME = "serialComRx"


def serial_com_read(serial_com_rx_device):
    """Read from serial communication device.

    Args:
        serial_com_rx_device (obj): Serial COM device

    Returns:
        Union[str, None]: Received data or None.
    """
    rx_data = None

    if serial_com_rx_device.getQueueLength() > 0:
        rx_data = serial_com_rx_device.getString()
        serial_com_rx_device.nextPacket()

    return rx_data


def rad_to_deg(angle_rad):
    """Convert angle in rad to degree.

    Args:
        angle_rad (float): Angle in rad

    Returns:
        float: Angle in degree
    """
    return angle_rad * 180 / math.pi


def convert_angle_to_2pi(angle):
    """Convert angle from range [-PI; PI] in rad to [0; 2PI].

    Args:
        angle (float): Angle in rad, range [-PI; PI].

    Returns:
        float: Angle in rad, range [0; 2PI]
    """
    if angle < 0:
        angle += 2 * math.pi

    return angle


def convert_webots_angle_to_ru_angle(webots_angle):
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


@dataclass
class OdometryData():
    """Odometry data container.
    """
    x: int = 0
    y: int = 0
    yaw_angle: float = 0

    def reset(self):
        """Reset odometry data.
        """
        self.x = 0
        self.y = 0
        self.yaw_angle = 0


# pylint: disable=too-many-branches
# pylint: disable=too-many-statements
# pylint: disable=too-many-locals
def main_loop():
    """Main loop:
        - Perform simulation steps until Webots is stopping the controller-

    Returns:
        number: Status
    """
    status = 0

    # Create the Supervisor instance.
    supervisor = Supervisor()

    # Get the time step of the current world.
    timestep = int(supervisor.getBasicTimeStep())

    # Enable supervisor receiver to receive any message from the robot or other
    # devices in the simulation.
    supervisor_com_rx = supervisor.getDevice(SUPERVISOR_RX_NAME)

    if supervisor_com_rx is None:
        print(
            f"No supervisor communication possible, because {SUPERVISOR_RX_NAME} not found.")
    else:
        supervisor_com_rx.enable(timestep)

    # Get robot node which to observe.
    robot_node = supervisor.getFromDef(ROBOT_NAME)

    if robot_node is None:

        print(f"Robot DEF {ROBOT_NAME} not found.")
        status = -1

    else:
        robot_observer = RobotObserver(robot_node)

        # Set current position and orientation in the map as reference.
        robot_observer.set_current_position_as_reference()
        robot_observer.set_current_orientation_as_reference()

        robot_position_old = robot_observer.get_rel_position()
        robot_orientation_old = robot_observer.get_rel_orientation()
        robot_odometry = OdometryData()

        while supervisor.step(timestep) != -1:
            if supervisor_com_rx is not None:
                rx_data = serial_com_read(supervisor_com_rx)

                if rx_data is None:
                    pass

                else:
                    command = rx_data.split(',')

                    # Reset robot position and orientation?
                    if command[0] == "RST":
                        print("RST")
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
                        print(f"Unknown command: {command[0]}")

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

                print(f"{int(robot_position[0])}, ", end="")
                print(f"{int(robot_position[1])}, ", end="")
                print(f"{int(rad_to_deg(yaw_ru_angle))} / ", end="")
                print(f"{robot_odometry.x}, ", end="")
                print(f"{robot_odometry.y}, ", end="")
                print(f"{int(rad_to_deg(robot_odometry.yaw_angle))}")

                robot_position_old = robot_position
                robot_orientation_old = robot_orientation

    return status


sys.exit(main_loop())
