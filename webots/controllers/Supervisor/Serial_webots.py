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
    Class for Serial Webots Communication
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
        
        bytes_sent = 0

        self.m_Emitter.send(bytes(payload))
        bytes_sent= len(payload)
        
        return bytes_sent

    def available(self) -> int:
        """ Check if there is anything available for reading

        Returns
        ----------
        Number of bytes available for reading.
        """
        return self.m_Receiver.getQueueLength()

    def read_bytes(self) -> tuple[int, bytearray]:
        """ Read a given number of Bytes from Serial.

        Returns
        ----------
        Tuple:
        - int: Number of bytes received.
        - bytearray: Received data.
        """
        rcvd_data = b''
        rcvd_data = self.m_Receiver.getBytes() 
           
        return len(rcvd_data), rcvd_data


    








################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################
