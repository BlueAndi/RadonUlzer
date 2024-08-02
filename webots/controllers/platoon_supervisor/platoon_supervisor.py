""" Platoon Supervisor controller for Webots. """

import sys
from controller import Supervisor, Keyboard, Node, Field  # pylint: disable=import-error


class Obstacle:
    """
    Class to represent the obstacle.
    """

    DEBOUNCE_TIME = 0.2  # Debounce time for the spacebar in seconds
    OBSTACLE_NAME = "OBSTACLE"  # Name of the obstacle node
    TRANSLATION_FIELD = "translation"  # Field name for the translation
    RAISED_HEIGHT = 0.4  # Active height in meters
    LOWERED_HEIGHT = 0.06  # Rest height in meters

    def __init__(self) -> None:
        """ Initialize the obstacle. """
        self.obstacle_node: Node = None
        self.translation_field: Field = None
        self.is_obstacle_lowered: bool = False
        self.last_time: float = 0

    def setup(self, supervisor: Supervisor) -> bool:
        """ Setup the obstacle. """
        is_successful: bool = False

        # Get the obstacle node
        self.obstacle_node = supervisor.getFromDef(self.OBSTACLE_NAME)

        if self.obstacle_node is not None:
            # Get the translation field of the obstacle node
            self.translation_field = self.obstacle_node.getField(
                self.TRANSLATION_FIELD)

            if self.translation_field is None:
                print(
                    f"Translation field {self.TRANSLATION_FIELD} \
                        of {self.OBSTACLE_NAME} not found.")
            else:
                # Get current time for debouncing
                self.last_time = supervisor.getTime()
                curr_pos = self.translation_field.getSFVec3f()
                self.translation_field.setSFVec3f([curr_pos[0],
                                                   curr_pos[1],
                                                   self.RAISED_HEIGHT])
                is_successful = True

        return is_successful

    def process(self, supervisor: Supervisor, keyboard: Keyboard) -> None:
        """ Process the obstacle. """

        if keyboard.getKey() == ord(' '):  # If space key is pressed
            if (supervisor.getTime() - self.last_time) > self.DEBOUNCE_TIME:  # Debounce space key
                # Update last time
                self.last_time = supervisor.getTime()

                # Get the current position of the obstacle
                curr_pos = self.translation_field.getSFVec3f()

                # Move the obstacle
                if self.is_obstacle_lowered is True:  # Raise the obstacle
                    print("Obstacle raised")
                    self.translation_field.setSFVec3f(
                        [curr_pos[0], curr_pos[1], self.RAISED_HEIGHT])
                else:  # Lower the obstacle
                    print("Obstacle lowered")
                    self.translation_field.setSFVec3f(
                        [curr_pos[0], curr_pos[1], self.LOWERED_HEIGHT])

                # Toggle the obstacle state
                self.is_obstacle_lowered = not self.is_obstacle_lowered


def main_loop():
    """Main loop:
        - Perform simulation steps until Webots is stopping the controller-

    Returns:
        number: Status
    """

    status = 0

    # Create the Supervisor instance.
    supervisor = Supervisor()

    # Get the keyboard device
    keyboard = supervisor.getKeyboard()

    # Create the obstacle instance
    obstacle = Obstacle()

    # Setup the obstacle
    if obstacle.setup(supervisor) is False:
        # No obstacle found.
        obstacle = None

    if status == 0:
        # Get the time step of the current world.
        timestep = int(supervisor.getBasicTimeStep())

        # Enable the keyboard device
        keyboard.enable(timestep)

        while supervisor.step(timestep) != -1:
            if obstacle is not None:
                obstacle.process(supervisor, keyboard)

    return status


if __name__ == "__main__":
    sys.exit(main_loop())
