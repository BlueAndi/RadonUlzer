""" Controller for Obstacle proto."""

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

import sys
from controller import Robot  # pylint: disable=import-error

################################################################################
# Variables
################################################################################

# Debounce time of collision in seconds.
DEBOUNCE_TIME = 1
TOUCH_SENSOR_NAME = 'touch'

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################


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

################################################################################
# Main
################################################################################


sys.exit(main_loop())
