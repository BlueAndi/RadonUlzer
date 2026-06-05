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

from typing import Optional
from controller import Supervisor  # pylint: disable=import-error

################################################################################
# Variables
################################################################################

################################################################################
# Classes
################################################################################

class MySupervisor(Supervisor):
    """My Supervisor class to extend the Supervisor class.
    This class is used to create a supervisor with custom serial communication channels.
    """

    def __init__(self, serial_com_rx_device_name: str,
                 serial_com_tx_device_name: str,
                 serial_com_rx_channel_id: int,
                 serial_com_tx_channel_id: int) -> None:
        """Initialize the MySupervisor class.

        Args:
            serial_com_rx_device_name (str): Name of the RX device.
            serial_com_tx_device_name (str): Name of the TX device.
            serial_com_rx_channel_id (int): ID of the RX channel.
            serial_com_tx_channel_id (int): ID of the TX channel.
        """
        super().__init__()

        # Rx/tx communication devices.
        self._com_rx = self.getDevice(serial_com_rx_device_name)
        self._com_tx = self.getDevice(serial_com_tx_device_name)

        # Simulation time step.
        self._timestep = int(self.getBasicTimeStep())

        if self._com_rx is None:
            print("Supervisor: No supervisor communication from others possible, "
                  f"because {serial_com_rx_device_name} not found.")
        else:
            self._com_rx.enable(self._timestep)
            self._com_rx.setChannel(serial_com_rx_channel_id)

        if self._com_tx is None:
            print("Supervisor: no supervisor communication to others possible, "
                  f"because {serial_com_tx_device_name} not found.")
        else:
            self._com_tx.setChannel(serial_com_tx_channel_id)

    def step(self) -> int:
        """Perform a simulation step.

        Returns:
            int: Status of the step, -1 if the simulation is stopped.
        """
        return super().step(self._timestep)

    def send(self, message: str) -> None:
        """Send a message to the serial communication device.

        Args:
            message (str): Message to send.
        """
        if self._com_tx is not None:
            self._com_tx.sendString(message)

    def receive(self) -> Optional[str]:
        """Receive a message from the serial communication device.

        Returns:
            str: Received message or None if no message is available.
        """
        received_string = None

        if self._com_rx is not None:
            if self._com_rx.getQueueLength() > 0:
                received_string = self._com_rx.getString()
                self._com_rx.nextPacket()

        return received_string

################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################
