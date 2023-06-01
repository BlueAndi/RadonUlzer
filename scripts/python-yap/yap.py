""" Yet Another Protocol (YAP) for lightweight communication. """

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

from dataclasses import dataclass
from struct import Struct
from socket_client import SocketClient

################################################################################
# Variables
################################################################################

################################################################################
# Classes
################################################################################


@dataclass(frozen=True)
class YAPConstants:
    """ Container Data class for YAP Constants """

    CHANNEL_LEN = 1  # Channel Field Length in Bytes
    DLC_LEN = 1  # DLC Field Length in Bytes
    CHECKSUM_LEN = 1  # Checksum Field Length in Bytes
    HEADER_LEN = CHANNEL_LEN + DLC_LEN + \
        CHECKSUM_LEN  # Length of Complete Header Field
    MAX_DATA_LEN = 32  # Data Field Length in Bytes
    MAX_FRAME_LEN = HEADER_LEN + MAX_DATA_LEN  # Total Frame Length in Bytes
    CHANNEL_NAME_MAX_LEN = 10  # Max length of channel name
    # Available Bytes in Control Channel Payload for data.
    CONTROL_CHANNEL_PAYLOAD_DATA_LENGTH = 4
    CONTROL_CHANNEL_CMD_BYTE_LENGTH = 1  # Lenght of Command in Bytes
    CONTROL_CHANNEL_NUMBER = 0  # Number of Control Channel.
    CONTROL_CHANNEL_PAYLOAD_LENGTH = CHANNEL_NAME_MAX_LEN + \
        CONTROL_CHANNEL_PAYLOAD_DATA_LENGTH + \
        CONTROL_CHANNEL_CMD_BYTE_LENGTH  # DLC of Heartbeat Command.
    # Index of the Command Byte of the Control Channel
    CONTROL_CHANNEL_COMMAND_INDEX = 0
    # Index of the start of the payload of the Control Channel
    CONTROL_CHANNEL_PAYLOAD_INDEX = 1
    HEATBEAT_PERIOD_SYNCED = 5000  # Period of Heartbeat when Synced.
    HEATBEAT_PERIOD_UNSYNCED = 1000  # Period of Heartbeat when Unsynced
    # Max number of attempts at receiving a Frame before resetting RX Buffer
    MAX_RX_ATTEMPTS = MAX_FRAME_LEN

    @dataclass
    class Commands():
        """ Enumeration of Commands of Control Channel. """
        SYNC = 0
        SYNC_RSP = 1
        SCRB = 2
        SCRB_RSP = 3

@dataclass
class Channel():
    """ Channel Definition """

    def __init__(self, channel_name: str = "", channel_dlc: int = 0, channel_callback=None) -> None:
        self.name = channel_name
        self.dlc = channel_dlc
        self.callback = channel_callback


class Frame():
    """ Frame Defintion """

    def __init__(self) -> None:
        self.raw = bytearray(YAPConstants.MAX_FRAME_LEN)
        self.channel = 0
        self.dlc = 0
        self.checksum = 0
        self.payload = bytearray(YAPConstants.MAX_DATA_LEN)

    def unpack_header(self):
        """ Unpack/parse channel number and checksum from raw frame """
        header_packer = Struct(">3B")
        self.channel, self.dlc, self.checksum = header_packer.unpack_from(
            self.raw)

    def unpack_payload(self):
        """ Unpack/parse payload from raw frame """
        self.payload = self.raw[3:]

    def pack_frame(self):
        """ Pack header and payload into raw array"""
        self.raw[0] = self.channel
        self.raw[1] = self.dlc
        self.raw[2] = self.checksum
        self.raw[3:] = self.payload

@dataclass
class ChannelArrays:
    """ Container Class for Channel Arrays and their counters """

    def __init__(self, max_configured_channels: int) -> None:
        self.number_of_rx_channels      = 0
        self.number_of_tx_channels      = 0
        self.number_of_pending_channels = 0
        self.rx_channels        = [Channel() for x in range(max_configured_channels)]
        self.tx_channels        = [Channel() for x in range(max_configured_channels)]
        self.pending_channels   = [Channel() for x in range(max_configured_channels)]


@dataclass
class SyncData:
    """ Container Dataclass for Synchronization Data. """

    def __init__(self) -> None:
        self.is_synced = False
        self.last_sync_response = 0
        self.last_sync_command = 0

@dataclass
class RxData:
    """ Container Dataclass for Receive Data and counters. """

    def __init__(self) -> None:
        self.received_bytes = 0
        self.rx_attempts = 0
        self.receive_frame = Frame()

class YAP:
    """ Yet Another Protocol YAP """

    def __init__(self, max_configured_channels: int, stream: SocketClient) -> None:
        self.__max_configured_channels = max_configured_channels
        self.__stream = stream
        self.__rx_data = RxData()
        self.__sync_data = SyncData()
        self.__channels = ChannelArrays(max_configured_channels)

    def process(self, current_timestamp: int) -> None:
        """Manage the Server functions.
        Call this function cyclic.

        Parameters
        ----------
        current_timestamp : int
            Time in milliseconds.

        """

        # Periodic Heartbeat.
        self.__heartbeat(current_timestamp)

        # Process RX data.
        self.__process_rx()

        # Process pending Subscriptions
        self.__process_subscriptions()

    def send_data(self, channel_name: str, payload: bytearray) -> bool:
        """Send a frame with the selected bytes.

        Parameters:
        ----------
        channel_name : str
            Channel to send frame to.
        payload : bytearray
            Byte buffer to be sent.

        Returns:
        --------
        If payload succesfully sent, returns true. Otherwise, false.
        """
        is_sent = False
        channel_number = self.get_tx_channel_number(channel_name)

        if (YAPConstants.CONTROL_CHANNEL_NUMBER != channel_number) and\
           (self.__sync_data.is_synced is True):

            is_sent = self.__send(channel_number, payload)

        return is_sent

    def is_synced(self) -> bool:
        """ Returns current Sync state of th YAP Server. """
        return self.__sync_data.is_synced

    def get_number_rx_channels(self) -> int:
        """Get the number of configured RX channels."""
        return self.__channels.number_of_rx_channels

    def get_number_tx_channels(self) -> int:
        """Get the number of configured TX channels."""
        return self.__channels.number_of_tx_channels

    def create_channel(self, name: str, dlc: int) -> int:
        """Creates a new TX Channel on the server.

        Parameters:
        ----------
        name : str
            Name of the channel. It will not be checked if the name already exists.
        dlc : int
            Length of the payload of this channel.

        Returns:
        --------
        The channel number if succesfully created, or 0 if not able to create new channel.
        """
        name_length = len(name)
        idx = 0

        if (0 != name_length) and\
           (YAPConstants.CHANNEL_NAME_MAX_LEN >= name_length) and\
           (0 != dlc) and\
           (YAPConstants.MAX_DATA_LEN >= dlc) and\
           (self.__max_configured_channels > self.__channels.number_of_tx_channels):

            # Create Channel
            self.__channels.tx_channels[self.__channels.number_of_tx_channels] = Channel(
                name, dlc)

            # Increase Channel Counter
            self.__channels.number_of_tx_channels += 1

            # Provide Channel Number. Could be summarized with operation above.
            idx = self.__channels.number_of_tx_channels

        return idx

    def subscribe_to_channel(self, name: str, callback) -> None:
        """ Suscribe to a Channel to receive the incoming data.

        Parameters:
        ----------
        name : str
            Name of the Channel to suscribe to.
        callback : function
            Callback to return the incoming data.
        """

        if (YAPConstants.CHANNEL_NAME_MAX_LEN >= len(name)) and\
           (callback is not None) and\
           (self.__max_configured_channels > self.__channels.number_of_pending_channels):

            # Save Name and Callback for channel creation after response
            self.__channels.pending_channels[self.__channels.number_of_pending_channels] = Channel(
                channel_name=name, channel_dlc=0, channel_callback=callback)

            # Increase Channel Counter
            self.__channels.number_of_pending_channels += 1

    def get_tx_channel_number(self, channel_name: str) -> int:
        """Get Number of a TX channel by its name.

        Parameters:
        -----------
        channel_name : str
            Name of channel

        Returns:
        --------
        Number of the Channel, or 0 if not channel with the name is present.
        """

        channel_number = 0
        for idx in range(self.__max_configured_channels):
            if self.__channels.tx_channels[idx].name == channel_name:
                channel_number = idx + 1
                break

        return channel_number

    def __heartbeat(self, current_timestamp: int) -> None:
        """ Periodic heartbeat.
            Sends SYNC Command depending on the current Sync state.

        Parameters
        ----------
        current_timestamp : int
            Time in milliseconds.

        """

        heartbeat_period = YAPConstants.HEATBEAT_PERIOD_UNSYNCED

        if self.__sync_data.is_synced is True:
            heartbeat_period = YAPConstants.HEATBEAT_PERIOD_SYNCED

        if (current_timestamp - self.__sync_data.last_sync_command) >= heartbeat_period:

            # Timeout
            if self.__sync_data.last_sync_command != self.__sync_data.last_sync_response:
                self.__sync_data.is_synced = False

            # Pack big-endian uint32
            packer = Struct(">L")
            timestamp_array = packer.pack(current_timestamp)

            # Send SYNC Command
            command = bytearray()
            command.append(YAPConstants.Commands.SYNC)
            command.extend(timestamp_array)

            # Pad array if necessary
            command = command.ljust(
                YAPConstants.CONTROL_CHANNEL_PAYLOAD_LENGTH, b'\x00')

            if self.__send(YAPConstants.CONTROL_CHANNEL_NUMBER, command) is True:
                self.__sync_data.last_sync_command = current_timestamp

    def __process_rx(self) -> None:
        """ Receive and process RX Data. """

        expected_bytes = 0
        dlc = 0

        # Determine how many bytes to read.
        if YAPConstants.HEADER_LEN > self.__rx_data.received_bytes:
            # Header must be read.
            expected_bytes = YAPConstants.HEADER_LEN - self.__rx_data.received_bytes
        else:
            # Header has been read. Get DLC of Rx Channel using header.
            self.__rx_data.receive_frame.unpack_header()
            # dlc = self.__get_channel_dlc(self.__receive_frame.channel, False)
            dlc = self.__rx_data.receive_frame.dlc

            # DLC = 0 means that the channel does not exist.
            if (0 != dlc) and (YAPConstants.MAX_RX_ATTEMPTS >= self.__rx_data.rx_attempts):
                remaining_payload_bytes = self.__rx_data.received_bytes - YAPConstants.HEADER_LEN
                expected_bytes = dlc - remaining_payload_bytes
                self.__rx_data.rx_attempts += 1

        # Are we expecting to read anything?
        if 0 != expected_bytes:

            # Read the required amount of bytes, if available.
            if self.__stream.available() >= expected_bytes:
                rcvd, data = self.__stream.read_bytes(expected_bytes)
                self.__rx_data.receive_frame.raw[self.__rx_data.received_bytes:] = data
                self.__rx_data.received_bytes += rcvd

            # Frame has been received.
            if (0 != dlc) and ((YAPConstants.HEADER_LEN + dlc) == self.__rx_data.received_bytes):

                # Check validity
                if self.__is_frame_valid(self.__rx_data.receive_frame) is True:
                    channel_array_index = self.__rx_data.receive_frame.channel - 1
                    self.__rx_data.receive_frame.unpack_payload()

                    # Differenciate between Control and Data Channels.
                    if YAPConstants.CONTROL_CHANNEL_NUMBER == self.__rx_data.receive_frame.channel:
                        self.__callback_control_channel(
                            self.__rx_data.receive_frame.payload)
                    elif self.__channels.rx_channels[channel_array_index].callback is not None:
                        self.__channels.rx_channels[channel_array_index].callback(
                            self.__rx_data.receive_frame.payload)

                # Frame received. Cleaning!
                self.__clear_local_buffers()
        else:
            # Invalid Header. Delete Frame!
            self.__clear_local_buffers()

    def __process_subscriptions(self) -> None:
        """ Subscribe to any pending Channels if synced to server. """

        # If synced and a channel is pending
        if (self.__sync_data.is_synced is True) and\
           (0 < self.__channels.number_of_pending_channels):
            # Channel Iterator
            for pending_channel in self.__channels.pending_channels:

                # Channel is pending
                if pending_channel.callback is not None:

                    # Subscribe to Channel
                    request = bytearray(
                        YAPConstants.CONTROL_CHANNEL_PAYLOAD_LENGTH)
                    request[0] = YAPConstants.Commands.SCRB
                    request[1:] = bytearray(pending_channel.name, 'ascii')

                    # Pad array if necessary
                    request = request.ljust(
                        YAPConstants.CONTROL_CHANNEL_PAYLOAD_LENGTH, b'\x00')

                    if self.__send(YAPConstants.CONTROL_CHANNEL_NUMBER, request) is False:
                        # Fall out of sync if failed to send.
                        self.__sync_data.is_synced = False
                        break

    def __clear_local_buffers(self) -> None:
        """ Clear Local RX Buffers """
        self.__rx_data.receive_frame = Frame()
        self.__rx_data.received_bytes = 0
        self.__rx_data.rx_attempts = 0

    def __get_tx_channel_dlc(self, channel_number: int) -> int:
        """ Get the Payload Length of a channel.

        Parameters
        ----------
        channel_number : int
            Channel number to check.

        is_tx_channel : bool
            Is the Channel a TX Channel? If false, will return value for an RX Channel instead.

        Returns
        -------
            DLC of the channel, or 0 if channel is not found.
        """
        dlc = 0

        if YAPConstants.CONTROL_CHANNEL_NUMBER == channel_number:
            dlc = YAPConstants.CONTROL_CHANNEL_PAYLOAD_LENGTH
        else:
            if self.__max_configured_channels >= channel_number:
                channel_idx = channel_number - 1
                dlc = self.__channels.tx_channels[channel_idx].dlc

        return dlc

    def __is_frame_valid(self, frame: Frame) -> bool:
        """ Check if a Frame is valid using its checksum.

        Parameters
        ----------
        frame : Frame
            Frame to be checked

        Returns:
            True if the Frame's checksum is correct. Otherwise, False.
        """

        return self.__checksum(frame.raw) == frame.checksum

    def __checksum(self, raw_frame: bytearray) -> int:
        """
        Performs simple checksum of the Frame to confirm its validity.

        Parameters:
        ----------
        raw_frame : Frame
            Frame to calculate checksum for

        Returns:
        -------
        Checksum value
        """
        newsum = raw_frame[0] + raw_frame[1]

        for idx in range(3, len(raw_frame)):
            newsum += raw_frame[idx]

        newsum = newsum % 255

        return newsum

    def __send(self, channel_number: int, payload: bytearray) -> bool:
        """ Send a frame with the selected bytes.

        Parameters:
        ----------
        channel_number : int
            Channel to send frame to.
        payload : bytearray
            Payload to send

        Returns:
        --------
        If payload succesfully sent, returns True. Otherwise, False.
        """

        frame_sent = False
        channel_dlc = self.__get_tx_channel_dlc(channel_number)

        if (len(payload) == channel_dlc) and \
           ((self.__sync_data.is_synced is True) or
                (YAPConstants.CONTROL_CHANNEL_NUMBER == channel_number)):
            written_bytes = 0
            new_frame = Frame()
            new_frame.channel = channel_number
            new_frame.dlc = channel_dlc
            new_frame.payload = payload
            new_frame.pack_frame()  # Pack Header and payload in Frame
            new_frame.checksum = self.__checksum(new_frame.raw)
            new_frame.pack_frame()  # Pack Checksum in Frame

            try:
                written_bytes = self.__stream.write(new_frame.raw)
            except Exception as excpt:  # pylint: disable=broad-exception-caught
                print(excpt)

            if written_bytes == (YAPConstants.HEADER_LEN + channel_dlc):
                frame_sent = True

        return frame_sent

    def __cmd_sync(self, payload: bytearray) -> None:
        """ Control Channel Command: SYNC

        Parameters:
        -----------
        payload : bytearray
            Command Data of received frame
        """

        response = bytearray(YAPConstants.CONTROL_CHANNEL_PAYLOAD_LENGTH)
        response[YAPConstants.CONTROL_CHANNEL_COMMAND_INDEX] = YAPConstants.Commands.SYNC_RSP
        response[YAPConstants.CONTROL_CHANNEL_PAYLOAD_INDEX:] = payload

        self.__send(YAPConstants.CONTROL_CHANNEL_NUMBER, response)

    def __cmd_sync_rsp(self, payload: bytearray) -> None:
        """ Control Channel Command: SYNC_RSP

        Parameters:
        -----------
        payload : bytearray
            Command Data of received frame
        """

        # Parse big-endian uint32
        unpacker = Struct(">L")
        rcvd_timestamp = unpacker.unpack_from(payload)[0]

        if self.__sync_data.last_sync_command == rcvd_timestamp:
            self.__sync_data.last_sync_response = self.__sync_data.last_sync_command
            self.__sync_data.is_synced = True
        else:
            self.__sync_data.is_synced = False

    def __cmd_scrb(self, payload: bytearray) -> None:
        """ Control Channel Command: SCRB

        Parameters:
        -----------
        payload : bytearray
            Command Data of received frame
        """

        response = bytearray(YAPConstants.CONTROL_CHANNEL_PAYLOAD_LENGTH)
        response[YAPConstants.CONTROL_CHANNEL_COMMAND_INDEX] = YAPConstants.Commands.SCRB_RSP

        # Parse name
        channel_name = str(payload, "ascii").strip('\x00')
        response[1] = self.get_tx_channel_number(channel_name)

        # Name is always sent back.
        response[2:] = bytearray(channel_name, 'ascii')

        # Pad array if necessary
        response = response.ljust(
            YAPConstants.CONTROL_CHANNEL_PAYLOAD_LENGTH, b'\x00')

        if self.__send(YAPConstants.CONTROL_CHANNEL_NUMBER, response) is False:
            # Fall out of sync if failed to send.
            self.__sync_data.is_synced = False

    def __cmd_scrb_rsp(self, payload: bytearray) -> None:
        """ Control Channel Command: SCRB

        Parameters:
        -----------
        payload : bytearray
            Command Data of received frame
        """

        # Parse payload
        channel_number = payload[0]
        channel_name = str(payload[1:], "ascii").strip('\x00')

        if (self.__max_configured_channels >= channel_number) and \
           (0 < self.__channels.number_of_pending_channels):

            # Channel Iterator
            for potential_channel in self.__channels.pending_channels:
                # Check if a SCRB is pending and is the correct channel
                if (potential_channel.callback is not None) and\
                   (potential_channel.name == channel_name):
                    # Channel is found in the server
                    if 0 != channel_number:
                        channel_array_index = channel_number - 1

                        if self.__channels.rx_channels[channel_array_index].callback is None:
                            self.__channels.number_of_rx_channels += 1

                        self.__channels.rx_channels[channel_array_index] = Channel(
                            channel_name, 0, potential_channel.callback)

                    # Channel is no longer pending
                    potential_channel = Channel()

                    # Decrease Channel Counter
                    self.__channels.number_of_pending_channels -= 1

                    # Break out of iterator
                    break

    def __callback_control_channel(self, payload: bytearray) -> None:
        """ Callback for the Control Channel

        Parameters:
        -----------
        payload : bytearray
            Payload of received frame
        """
        if len(payload) != YAPConstants.CONTROL_CHANNEL_PAYLOAD_LENGTH:
            return

        cmd_byte = payload[YAPConstants.CONTROL_CHANNEL_COMMAND_INDEX]
        cmd_data = payload[YAPConstants.CONTROL_CHANNEL_PAYLOAD_INDEX:]

        # Switch not available until Python 3.10
        # Find command handler
        if YAPConstants.Commands.SYNC == cmd_byte:
            self.__cmd_sync(cmd_data)
        elif YAPConstants.Commands.SYNC_RSP == cmd_byte:
            self.__cmd_sync_rsp(cmd_data)
        elif YAPConstants.Commands.SCRB == cmd_byte:
            self.__cmd_scrb(cmd_data)
        elif YAPConstants.Commands.SCRB_RSP == cmd_byte:
            self.__cmd_scrb_rsp(cmd_data)

################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################
