""" Implementation of a Serial Client for Serial Communication """

# MIT License
#
# Copyright (c) 2023 Andreas Merkle <web@blue-andi.de>
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
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#


################################################################################
# Imports
################################################################################

import serial

################################################################################
# Variables
################################################################################

################################################################################
# Classes
################################################################################


class SerialClient:
    """
    Class for Serial Communication
    """

    def __init__(self, port_name: str, port_speed: int) -> None:
        """ SerialClient Constructor.

        Parameters
        ----------
        port_name : str
            Name of Serial Port.
        port_speed : int
            Speed of Serial Port.
        """

        self.__port_name = port_name
        self.__port_speed = port_speed
        self.__serial = serial.Serial(port_name, port_speed, timeout=1)

    def connect_to_server(self) -> None:
        """ Connect to the specified Host and Port. """

        try:
            # Clean Serial before starting
            available_bytes = self.available()
            if available_bytes > 0:
                self.read_bytes(available_bytes)
        except:
            print(
                f"Failed to connect to \"{self.__port_name}\" on port {self.__port_speed}")
            raise

    def disconnect_from_server(self) -> None:
        """ Close the connection to Server. """

        self.__serial.close()

    def write(self, payload : bytearray) -> int:
        """ Sends Data to the Server.

        Parameters
        ----------
        payload : bytearray
            Payload to send.

        Returns
        ----------
        Number of bytes sent
        """

        bytes_sent = 0

        try:
            bytes_sent = self.__serial.write(payload)
        except BlockingIOError as err:
            raise err

        return bytes_sent

    def available(self) -> int:
        """ Check if there is anything available for reading

        Returns
        ----------
        Number of bytes available for reading.
        """

        rcvd_data = b''
        try:
            rcvd_data = self.__serial.in_waiting
        except BlockingIOError:
            # Exception thrown on non-blocking Serial when there is nothing to read.
            pass

        return rcvd_data

    def read_bytes(self, length: int) -> tuple[int, bytearray]:
        """ Read a given number of Bytes from Serial.

        Parameters
        ----------
        lenght : int
            Number of bytes to read.

        Returns
        ----------
        Tuple:
        - int: Number of bytes received.
        - bytearray: Received data.
        """

        rcvd_data = b''
        try:
            rcvd_data = self.__serial.read(length)
        except BlockingIOError:
            # Exception thrown on non-blocking Serial when there is nothing to read.
            pass

        return len(rcvd_data),  rcvd_data


################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################
