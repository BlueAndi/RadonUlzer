""" Controller for Obstacle proto."""

import sys
from controller import Robot  # pylint: disable=import-error

# Debounce time of collision in seconds.
DEBOUNCE_TIME = 1
TOUCH_SENSOR_NAME = 'touch'


def main_loop():
    """Main loop:
        - Perform simulation steps until Webots is stopping the controller-

    Returns:
        number: Status
    """
    status = 0

    # Create robot instance.
    obstacle = Robot()

    # Get touch sensor device to detect collision.
    # Touch sensor also triggered when clicking the obstacle in the Webots world.
    touch_sensor = obstacle.getDevice(TOUCH_SENSOR_NAME)

    if touch_sensor is None:
        print(f'Touch sensor "{TOUCH_SENSOR_NAME}" not found!')
        status = -1
    else:
        # Get basic time step of the current world.
        timestep = int(obstacle.getBasicTimeStep())

        touch_sensor.enable(timestep)  # Enable touch sensor
        last_collision_time = obstacle.getTime()  # Get current time for debouncing

        print('Obstacle ready!')

        while -1 != obstacle.step(timestep):  # Main loop

            if 0 < touch_sensor.getValue():  # If collision detected
                if (obstacle.getTime() - last_collision_time) > DEBOUNCE_TIME:  # Debounce collision

                    # Update last collision time
                    last_collision_time = obstacle.getTime()

                    # On (debounced) collision detected:
                    print('Collision Detected!')

    return status


sys.exit(main_loop())
