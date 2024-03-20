"""Supervisor controller.
    Details: https://github.com/cyberbotics/webots/blob/master/docs/guide/supervisor-programming.md
"""

import math
import sys
from controller import Supervisor
from robot_observer import RobotObserver

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

class OdometryData(): # pylint: disable=too-few-public-methods
    """Odometry data container.
    """
    def __init__(self) -> None:
        self.x = 0
        self.y = 0
        self.yaw_angle = 0

    def reset(self):
        """Reset odometry data.
        """
        self.x = 0
        self.y = 0
        self.yaw_angle = 0

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
        print(f"No supervisor communication possible, because {SUPERVISOR_RX_NAME} not found.")
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
                        robot_odometry.x = int(rx_data[1]) # [mm]
                        robot_odometry.y = int(rx_data[2]) # [mm]
                        robot_odometry.yaw_angle = float(rx_data[3]) / 1000.0 # [rad]

                    # Unknown command.
                    else:
                        print(f"Unknown command: {command[0]}")

            robot_position = robot_observer.get_rel_position()
            robot_orientation = robot_observer.get_rel_orientation()

            any_change = has_position_changed(robot_position, robot_position_old)

            if any_change is False:
                any_change = has_orientation_changed(robot_orientation, robot_orientation_old)

            if any_change is True:
                print(f"{int(robot_position[0])}, ", end="")
                print(f"{int(robot_position[1])}, ", end="")
                print(f"{int(robot_position[2])}, ", end="")
                print(f"{int(rad_to_deg(robot_orientation[2]))} / ", end="")
                print(f"{robot_odometry.x}, ", end="")
                print(f"{robot_odometry.y}, ", end="")
                print(f"{int(rad_to_deg(robot_odometry.yaw_angle))}")

                robot_position_old = robot_position
                robot_orientation_old = robot_orientation

    return status

sys.exit(main_loop())
