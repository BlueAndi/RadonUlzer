""" Platoon Supervisor controller for Webots. Moves the obstacle up and down when the space key is pressed. """

import sys
from controller import Supervisor

DEBOUNCE_TIME = 0.2  # Debounce time for the spacebar in seconds
OBSTACLE_NAME = "OBSTACLE"  # Name of the obstacle node
TRANSLATION_FIELD = "translation"  # Field name for the translation

RAISED_HEIGHT = 0.4  # Active height in meters
LOWERED_HEIGHT = 0.06  # Rest height in meters

# Create the Supervisor instance.
supervisor = Supervisor()

# Get the keyboard device
keyboard = supervisor.getKeyboard()


def main_loop():
    """Main loop:
        - Perform simulation steps until Webots is stopping the controller-

    Returns:
        number: Status
    """

    # Get the obstacle node
    obstacle_node = supervisor.getFromDef(OBSTACLE_NAME)

    if obstacle_node is None:

        print(f"Robot DEF {OBSTACLE_NAME} not found.")
        status = -1

    else:
        # Get the translation field of the obstacle node
        translation_field = obstacle_node.getField(TRANSLATION_FIELD)

        if translation_field is None:
            print(
                f"Translation field {TRANSLATION_FIELD} of {OBSTACLE_NAME} not found.")
            status = -1

        else:
            # Get the time step of the current world.
            timestep = int(supervisor.getBasicTimeStep())

            # Enable the keyboard device
            keyboard.enable(timestep)

            # Get current time for debouncing
            last_time = supervisor.getTime()

            # Set the initial state to raised obstacle
            is_obstacle_lowered = False
            curr_pos = translation_field.getSFVec3f()
            translation_field.setSFVec3f(
                [curr_pos[0], curr_pos[1], RAISED_HEIGHT])

            while supervisor.step(timestep) != -1:
                if keyboard.getKey() == ord(' '):  # If space key is pressed
                    if (supervisor.getTime() - last_time) > DEBOUNCE_TIME:  # Debounce space key

                        # Update last time
                        last_time = supervisor.getTime()

                        # Get the current position of the obstacle
                        curr_pos = translation_field.getSFVec3f()

                        # Move the obstacle
                        if is_obstacle_lowered is True:  # Raise the obstacle
                            translation_field.setSFVec3f(
                                [curr_pos[0], curr_pos[1], RAISED_HEIGHT])
                        else:  # Lower the obstacle
                            translation_field.setSFVec3f(
                                [curr_pos[0], curr_pos[1], LOWERED_HEIGHT])

                        # Toggle the obstacle state
                        is_obstacle_lowered = not is_obstacle_lowered

    return status


sys.exit(main_loop())
