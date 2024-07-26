################################################################################
# Imports
################################################################################
from controller import device
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
        """
        Check if there is anything available for reading.

        Returns
        ----------
        int
            Number of bytes that are available for reading.
        """
        if len(self.__buffer) > 0:
            return len(self.__buffer)
        elif self.__receiver.getQueueLength() > 0:
            return self.__receiver.getDataSize()
        return 0

    def read_bytes(self, length: int) -> tuple[int, bytearray]:
        """
        Read a given number of Bytes from Serial.

        Returns
        ----------
        tuple[int, bytearray]
            - int: Number of bytes received.
            - bytearray: Received data.
        """
        read = 0
        data = bytearray()

        if len(self.__buffer) > 0:
            read = min(len(self.__buffer), length)
            data = self.__buffer[:read]
            self.__buffer = self.__buffer[read:]
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
