"""Supervisor controller.
    Details: https://github.com/cyberbotics/webots/blob/master/docs/guide/supervisor-programming.md
"""

import math
import sys
import numpy as np
from controller import Supervisor

# Create the Supervisor instance.
supervisor = Supervisor()

# The PROTO DEF name must be given!
ROBOT_NAME = "ROBOT"

def get_robot_translation_values(robot_node):
    """Get robot translation field values.

    Args:
        robot_node (robot): The robot node.

    Returns:
        list: List of translation field values for x, y and z.
    """
    robot_field_translation = robot_node.getField("translation")
    return robot_field_translation.getSFVec3f()

def get_robot_rotation_values(robot_node):
    """Get robot rotation field values.

    Args:
        robot_node (robot): The robot node.

    Returns:
        list: List of rotation field values for x, y, z and angle.
    """
    robot_field_rotation = robot_node.getField("rotation")
    return robot_field_rotation.getSFRotation()

def get_robot_3d_rel_position(robot_node, prev_translation_values):
    """Get robot 3d relative position (x, y).

    Args:
        robot_node (robot): The robot node.
        prev_translation_values (list): Absolute translation values to calculate the relative from.

    Returns:
        list: List with position (x, y, z) in [mm]
    """
    vector_idx_x = 0
    vector_idx_y = 1
    vector_idx_z = 2
    factor_m_to_mm = 1000 # Conversion factor from [m] to [mm]

    robot_translation_values = get_robot_translation_values(robot_node)

    rel_pos_x = ( robot_translation_values[vector_idx_x] - prev_translation_values[vector_idx_x] ) \
                * factor_m_to_mm # [mm]
    rel_pos_y = ( robot_translation_values[vector_idx_y] - prev_translation_values[vector_idx_y] ) \
                * factor_m_to_mm # [mm]
    rel_pos_z = ( robot_translation_values[vector_idx_z] - prev_translation_values[vector_idx_z] ) \
                * factor_m_to_mm # [mm]

    return rel_pos_x, rel_pos_y, rel_pos_z

def get_robot_2d_orientation(robot_node):
    """Get the robot 2d orientation angle [-PI, PI].

    Args:
        robot_node (Robot): The robot node.

    Returns:
        float: Orientation angle in [rad]
    """
    vector_idx_angle = 3

    robot_rotation_values = get_robot_rotation_values(robot_node)

    return robot_rotation_values[vector_idx_angle]

def main_loop():
    """Main loop:
        - Perform simulation steps until Webots is stopping the controller-

    Returns:
        number: Status
    """
    status = 0

    # Get the time step of the current world.
    timestep = int(supervisor.getBasicTimeStep())

    # Get robot node which to observe.
    robot_node = supervisor.getFromDef(ROBOT_NAME)

    if robot_node is None:

        print(f"Robot DEF {ROBOT_NAME} not found.")
        status = -1

    else:

        # Get robot absolute start position
        robot_start_translation_values = get_robot_translation_values(robot_node)

        prev_pos_x = -1
        prev_pos_y = -1
        prev_orientation_deg = -1

        while supervisor.step(timestep) != -1:
            pos_x, pos_y, _ = get_robot_3d_rel_position(robot_node, robot_start_translation_values)
            orientation = get_robot_2d_orientation(robot_node)
            orientation_deg = math.degrees(orientation)

            # Only interested in the floor part
            pos_x = int(pos_x)
            pos_y = int(pos_y)
            orientation_deg = int(orientation_deg)

            # pylint: disable=line-too-long
            if (prev_pos_x != pos_x) or (prev_pos_y != pos_y) or (prev_orientation_deg != orientation_deg):
                print(f"{pos_x}, {pos_y}, {orientation_deg}\n")

                prev_pos_x = pos_x
                prev_pos_y = pos_y
                prev_orientation_deg = orientation_deg

    return status

sys.exit(main_loop())
