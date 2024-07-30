""" Implementation of a Serial Webots for Serial Communication """

# MIT License
#
# Copyright (c) 2023 - 2024 Gabryel Reyes <gabryelrdiaz@gmail.com>
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

from controller import device  # pylint: disable=import-error
from SerialMuxProt import Stream

################################################################################
# Classes
################################################################################


class SerialWebots(Stream):
    """
    Serial Webots Communication Class
    """

    def __init__(self, emitter: device, receiver: device) -> None:
        """
        SerialWebots Constructor.

        Parameters
        ----------
        emitter : device
            Name of Emitter Device
        receiver : device
            Name of Receiver Device
        """
        self.__emitter = emitter
        self.__receiver = receiver
        self.__buffer = bytearray()

    def write(self, payload: bytearray) -> int:
        """
        Sends Data to the Server.

        Parameters
        ----------
        payload : bytearray
            Payload to send.

        Returns
        ----------
        int
            Number of bytes sent
        """
        self.__emitter.send(bytes(payload))
        bytes_sent = len(payload)

        return bytes_sent

    def available(self) -> int:
        """Check if there is anything available for reading.

        Returns
        ----------
        Number of bytes that are available for reading.
        """

        if len(self.__buffer) > 0:
            return len(self.__buffer)

        if self.__receiver.getQueueLength() > 0:
            return self.__receiver.getDataSize()

        return 0

    def read_bytes(self, length: int) -> tuple[int, bytearray]:
        """Read a given number of Bytes from Serial.

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

        read = 0
        data = bytearray()

        # Check if there is data in the buffer. Read the specified Bytes.
        if len(self.__buffer) > 0:
            read = min(len(self.__buffer), length)
            data = self.__buffer[:read]
            self.__buffer = self.__buffer[read:]

        # Check if there is data in the receiver queue and process it
        elif self.__receiver.getQueueLength() > 0:
            received_data = self.__receiver.getBytes()
            received_data_size = self.__receiver.getDataSize()
            self.__receiver.nextPacket()

            if received_data_size > length:
                data = received_data[:length]
                self.__buffer = received_data[length:]
                read = length
            else:
                data = received_data
                read = received_data_size

        return read, data


################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################
