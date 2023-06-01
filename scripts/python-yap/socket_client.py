""" Implementation of a Socket Client for Inter-Process Communication """

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

import socket

################################################################################
# Variables
################################################################################

################################################################################
# Classes
################################################################################


class SocketClient:
    """
    Class for Socket Communication
    """

    def __init__(self, server_address: str, port_number: int) -> None:
        """ SocketClient Constructor.

        Parameters
        ----------
        server_address : str
            Address of the Server to connect to.
        port_number : int
            Port of of Server.
        """

        self.__server_address = server_address
        self.__port_number = port_number
        self.__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def connect_to_server(self) -> None:
        """ Connect to the specified Host and Port. """

        try:
            self.__socket.connect((self.__server_address, self.__port_number))
            self.__socket.setblocking(False)

            # Clean socket before starting
            available_bytes = self.available()
            if available_bytes > 0:
                self.read_bytes(available_bytes)
        except:
            print(
                f"Failed to connect to \"{self.__server_address}\" on port {self.__port_number}")
            raise

    def disconnect_from_server(self) -> None:
        """ Close the connection to Server. """

        self.__socket.close()

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
            bytes_sent = self.__socket.send(payload)
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
            rcvd_data = self.__socket.recv(2048, socket.MSG_PEEK)
        except BlockingIOError:
            # Exception thrown on non-blocking socket when there is nothing to read.
            pass

        return len(rcvd_data)

    def read_bytes(self, length: int) -> tuple[int, bytearray]:
        """ Read a given number of Bytes from Socket.

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
            rcvd_data = self.__socket.recv(length)
        except BlockingIOError:
            # Exception thrown on non-blocking socket when there is nothing to read.
            pass

        return len(rcvd_data),  rcvd_data


################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################
