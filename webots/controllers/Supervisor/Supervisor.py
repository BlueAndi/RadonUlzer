"""Supervisor controller.
    Details: https://github.com/cyberbotics/webots/blob/master/docs/guide/supervisor-programming.md
"""
import sys
from Serial_webots import SerialWebots
from SerialMuxProt import SerialMuxProt
from controller import Supervisor
import struct

# The PROTO DEF name must be given!
ROBOT_NAME = "ROBOT"

# The supervisor receiver name.
SUPERVISOR_RX_NAME = "serialComRx"

# The supervisor Emitter name.
SUPERVISOR_TX_NAME = "serialComTx"

# Create the Supervisor instance.
supervisor = Supervisor()

# Get the time step of the current world.
timestep = int(supervisor.getBasicTimeStep())

# Get the Supervisor Receiver Device
supervisor_com_rx = supervisor.getDevice(SUPERVISOR_RX_NAME)

# Get the Supervisor Emitter device 
supervisor_com_Tx = supervisor.getDevice(SUPERVISOR_TX_NAME)

# Set serial rx/tx channels for communication with the Roboter.
supervisor_com_rx.setChannel(2)
supervisor_com_Tx.setChannel(1)

# Enable supervisor receiver to receive any message from the robot or other
# devices in the simulation.
if supervisor_com_rx is None:
    print(f"No supervisor communication possible, because {SUPERVISOR_RX_NAME} not found.")
else:
    supervisor_com_rx.enable(timestep)

# Get robot node which to observe.
robot_node = supervisor.getFromDef(ROBOT_NAME)

# SerialMuxProt Server Instance.
S_client = SerialWebots(supervisor_com_Tx, supervisor_com_rx)
smp_server = SerialMuxProt(10, S_client) 

# The PROTO DEF name must be given!
ROBOT_NAME = "ROBOT"

#DLC of SPEED_SET Channel
SPEED_SET_DLC = 4

m_serialMuxProtChannelIdSPEED_SET = smp_server.create_channel("SPEED_SET", SPEED_SET_DLC)

# Counter for the number of times no line has been detected
noLineDetectionCount = 0

# The line sensor threshold
LINE_SENSOR_ON_TRACK_MIN_VALUE = 200

def callback_Status(payload: bytearray) -> int:
    """ Callback Status Channel """
    if payload[0] == 1:
       print("the max. time within the robot must be finished its drive is  Done")
       smp_server.send_data("SPEED_SET", struct.pack('2H', 0, 0))  # Stop motors


def callback_LineSensors(payload: bytearray)-> None:
    """ Callback LINE_SENS Channel"""
    global noLineDetectionCount,LINE_SENSOR_ON_TRACK_MIN_VALUE
    sensor_data = struct.unpack('5H', payload)
    for idx in range (5):
         print(f"Sensor[{idx}] = {sensor_data[idx]}")
    if all(value == 0 for value in sensor_data):
        noLineDetectionCount += 1
    else:
        noLineDetectionCount = 0
    
    if noLineDetectionCount >= 20 or all(value >= LINE_SENSOR_ON_TRACK_MIN_VALUE for value in sensor_data):
        smp_server.send_data("SPEED_SET", struct.pack('2H', 0, 0))  # Stop motors, maximum NO Line Detection Counter reached   
    else:
        smp_server.send_data("SPEED_SET", struct.pack('2H', 1000, 1000)) 
    
def callback_Mode(payload: bytearray)-> None:
    """ Callback MODE Channel"""
    driving_mode = payload[0]
    if driving_mode:
        print("Driving Mode Selected")
    else:
        print("Train Mode Selected")    

smp_server.subscribe_to_channel("STATUS", callback_Status)
smp_server.subscribe_to_channel("LINE_SENS",callback_LineSensors)
smp_server.subscribe_to_channel("MODE",callback_Mode)
    
def main_loop():
    """Main loop:
        - Perform simulation steps until Webots is stopping the controller-

    Returns:
        number: Status
    """
    status = 0
    m_elapsedTimeSinceReset = 0

    if m_serialMuxProtChannelIdSPEED_SET == 0:
        print("Channel SPEED_SET not created.")
    else:
        # Get robot node which to observe.
        robot_node = supervisor.getFromDef(ROBOT_NAME)
        if robot_node is None:
            print(f"Robot DEF {ROBOT_NAME} not found.")
            status = -1         
        else:
             while supervisor.step(timestep) != -1:
                   m_elapsedTimeSinceReset += timestep
                   smp_server.process(m_elapsedTimeSinceReset)
    return status

sys.exit(main_loop())

