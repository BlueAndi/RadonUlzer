################################################################################
# Imports
################################################################################
from controller import device
################################################################################
# Variables
################################################################################

################################################################################
# Classes
################################################################################


class SerialWebots:
    """
    Serial Webots Communication Class
    """

    def __init__(self, Emitter: device, Receiver: device)-> None:
        """ SerialWebots Constructor.

         Parameters
        ----------
        Emitter : device
            Name of Emitter Device
        Receiver : device
            Name of Receiver Device

        """
        self.m_Emitter = Emitter
        self.m_Receiver = Receiver
        self.buffer  = bytearray()

        
    def write(self, payload : bytearray ) -> int:
        """ Sends Data to the Server.

        Parameters
        ----------
        payload : bytearray
            Payload to send.

        Returns
        ----------
        Number of bytes sent
        """
        self.m_Emitter.send(bytes(payload))
        bytes_sent= len(payload)
        
        return bytes_sent

    def available(self) -> int:
        """ Check if there is anything available for reading

        Returns
        ----------
        Number of bytes that are available for reading.
        
        """
        if len(self.buffer) > 0:
            return len(self.buffer)
        elif self.m_Receiver.getQueueLength() > 0:
            return self.m_Receiver.getDataSize()
        return 0 

    def read_bytes(self, length: int) -> tuple[int, bytearray]:
        """ Read a given number of Bytes from Serial.

        Returns
        ----------
        Tuple:
        - int: Number of bytes received.
        - bytearray: Received data.
        """

        read = 0
        data = bytearray()

        if len(self.buffer) > 0:
            read = min(len(self.buffer), length)
            data = self.buffer[:read]
            self.buffer = self.buffer[read:]
        elif self.m_Receiver.getQueueLength() > 0:
            receivedData = self.m_Receiver.getBytes()
            receivedData_size = self.m_Receiver.getDataSize()
            self.m_Receiver.nextPacket()

            if receivedData_size > length:
                data = receivedData[:length]
                self.buffer = receivedData[length:]
                read = length
            else:
                data = receivedData
                read = receivedData_size

        return read, data


    








################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################
