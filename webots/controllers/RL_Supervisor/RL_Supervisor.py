"""
Supervisor controller.

Details: https://github.com/cyberbotics/webots/blob/master/docs/guide/supervisor-programming.md
"""

import sys
import struct
from controller import Supervisor  # pylint: disable=import-error
from Serial_webots import SerialWebots
from SerialMuxProt import Server

# Constants
ROBOT_NAME = "ROBOT"
SUPERVISOR_RX_NAME = "serialComRx"
SUPERVISOR_TX_NAME = "serialComTx"
COMMAND_CHANNEL_NAME = "CMD"
CMD_DLC = 1
SPEED_SETPOINT_CHANNEL_NAME = "SPEED_SET"
SPEED_SET_DLC = 4
LINE_SENSOR_CHANNEL_NAME = "LINE_SENS"
STATUS_CHANNEL_NAME = "STATUS"
MODE_CHANNEL_NAME = "MODE"
CMD_ID_SET_READY_STATE = 1
LINE_SENSOR_ON_TRACK_MIN_VALUE = 200
POSITION_DATA = [-0.24713614078815466, -0.04863962992854465, 0.013994298332013683]
ORIENTATION_DATA = [
    -1.0564747468923541e-06,
    8.746699709178704e-07,
    0.9999999999990595,
    1.5880805820884731,
]

# Counter for the number of consecutive times a line has not been detected.
no_line_detection_count = [0]

# The counter of the number of times it has been reset.
reset_count = [0]

# Create the Supervisor instance.
supervisor = Supervisor()

# Get the time step of the current world.
timestep = int(supervisor.getBasicTimeStep())

# Get the Supervisor Receiver Device
supervisor_com_rx = supervisor.getDevice(SUPERVISOR_RX_NAME)

# Get the Supervisor Emitter device
supervisor_com_tx = supervisor.getDevice(SUPERVISOR_TX_NAME)

# Set serial rx/tx channels for communication with the Roboter.
supervisor_com_rx.setChannel(2)
supervisor_com_tx.setChannel(1)

# Get robot node which to observe.
robot_node = supervisor.getFromDef(ROBOT_NAME)

# SerialMuxProt Server Instance.
s_client = SerialWebots(supervisor_com_tx, supervisor_com_rx)
smp_server = Server(10, s_client)

# Channel creation.
serialMuxProtChannelIdSPEED_SET = smp_server.create_channel(
    SPEED_SETPOINT_CHANNEL_NAME, SPEED_SET_DLC
)
serialMuxProtChannelIdCMD = smp_server.create_channel(COMMAND_CHANNEL_NAME, CMD_DLC)

# Enable supervisor receiver to receive any message from the robot or other
# devices in the simulation.
if supervisor_com_rx is None:
    print(
        f"No supervisor communication possible, because {SUPERVISOR_RX_NAME} not found."
    )
else:
    supervisor_com_rx.enable(timestep)


def reinitialize():
    """Re-initialization of position and orientation of The ROBOT."""
    trans_field = robot_node.getField("translation")
    rot_field = robot_node.getField("rotation")
    intial_position = POSITION_DATA
    initial_orientation = ORIENTATION_DATA
    trans_field.setSFVec3f(intial_position)
    rot_field.setSFRotation(initial_orientation)
    reset_count[0] = 0


def callback_status(payload: bytearray) -> int:
    """Callback Status Channel."""
    if payload[0] == 1:
        print("the max. time within the robot must be finished its drive is  Done")
        smp_server.send_data(
            SPEED_SETPOINT_CHANNEL_NAME, struct.pack("2H", 0, 0)
        )  # Stop motors
        smp_server.send_data(
            COMMAND_CHANNEL_NAME, struct.pack("B", CMD_ID_SET_READY_STATE)
        )  # SENDING A COMMAND_ID SET READY STATUS
        reinitialize()


def callback_line_sensors(payload: bytearray) -> None:
    """Callback LINE_SENS Channel."""
    # sensor_data now contains a tuple of 5 unsigned short integers extracted from the payload
    sensor_data = struct.unpack("5H", payload)

    for idx in range(5):
        if idx == 4:
            print(f"Sensor[{idx}] = {sensor_data[idx]}")
        else:
            print(f"Sensor[{idx}] = {sensor_data[idx]}, ", end="")

    if all(value == 0 for value in sensor_data):
        no_line_detection_count[0] += 1
    else:
        no_line_detection_count[0] = 0

    if no_line_detection_count[0] >= 20 or all(
        value >= LINE_SENSOR_ON_TRACK_MIN_VALUE for value in sensor_data
    ):

        # Stop motors, maximum NO Line Detection Counter reached
        smp_server.send_data(SPEED_SETPOINT_CHANNEL_NAME, struct.pack("2H", 0, 0))

        # SENDING A COMMAND ID SET READY STATUS
        smp_server.send_data(
            COMMAND_CHANNEL_NAME, struct.pack("B", CMD_ID_SET_READY_STATE)
        )
        reset_count[0] += 1

    else:
        # Set left and right motors speed to 1000
        smp_server.send_data(SPEED_SETPOINT_CHANNEL_NAME, struct.pack("2H", 1000, 1000))


def callback_mode(payload: bytearray) -> None:
    """Callback MODE Channel."""
    driving_mode = payload[0]

    if driving_mode:
        print("Driving Mode Selected.")
    else:
        print("Train Mode Selected.")


def main_loop():
    """Main loop:
        - Perform simulation steps until Webots is stopping the controller-

    Returns:
        number: Status
    """
    status = 0
    elapsed_time_since_reset = 0  # Elapsed time since reset [ms]

    # Channel subscription.
    smp_server.subscribe_to_channel(STATUS_CHANNEL_NAME, callback_status)
    smp_server.subscribe_to_channel(LINE_SENSOR_CHANNEL_NAME, callback_line_sensors)
    smp_server.subscribe_to_channel(MODE_CHANNEL_NAME, callback_mode)

    if serialMuxProtChannelIdSPEED_SET == 0 or serialMuxProtChannelIdCMD == 0:
        print("Channel SPEED_SET not created.")
    else:

        if robot_node is None:
            print(f"Robot DEF {ROBOT_NAME} not found.")
            status = -1

        else:
            while supervisor.step(timestep) != -1:
                elapsed_time_since_reset += timestep
                smp_server.process(elapsed_time_since_reset)

                if reset_count[0] != 0:
                    reinitialize()
    return status


sys.exit(main_loop())
