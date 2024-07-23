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

#DLC of SPEED_SET Channel
SPEED_SET_DLC = 4
#DLC of STATUS Channel
STATUS_DLC = 1

# Counter for the number of times no line has been detected
noLineDetectionCount = 0

# The line sensor threshold.
Line_Sensor_On_Track_Min_Value = 200

# The counter of how much it has been reset.
Reset_Count = 0

def reinitialize ():
    """ Re-initialization of position and orientation of The ROBOT """
    global robot_node,Reset_Count
    trans_field = robot_node.getField("translation")
    rot_field = robot_node.getField("rotation")
    intial_position =[-0.24713614078815466, -0.04863962992854465, 0.013994298332013683]
    initial_orientation =[-1.0564747468923541e-06, 8.746699709178704e-07, 0.9999999999990595, 1.5880805820884731]
    trans_field.setSFVec3f(intial_position)
    rot_field.setSFRotation(initial_orientation)
    Reset_Count = 0

def callback_Status(payload: bytearray) -> int:
    """ Callback Status Channel """
    if payload[0] == 1:
       print("the max. time within the robot must be finished its drive is  Done")
       smp_server.send_data("SPEED_SET", struct.pack('2H', 0, 0))  # Stop motors
       smp_server.send_data("STATUS",struct.pack('B', 1))
       reinitialize()


def callback_LineSensors(payload: bytearray)-> None:
    """ Callback LINE_SENS Channel"""
    global noLineDetectionCount, Line_Sensor_On_Track_Min_Value, Reset_Count
    sensor_data = struct.unpack('5H', payload)
    for idx in range (5):
         if idx == 4:
              print(f"Sensor[{idx}] = {sensor_data[idx]}")
         else :
               print(f"Sensor[{idx}] = {sensor_data[idx]}, ", end="")
               
    if all(value == 0 for value in sensor_data):
        noLineDetectionCount += 1
    else:
        noLineDetectionCount = 0
    
    if noLineDetectionCount >= 20 or all(value >= Line_Sensor_On_Track_Min_Value for value in sensor_data):
        smp_server.send_data("SPEED_SET", struct.pack('2H', 0, 0))  # Stop motors, maximum NO Line Detection Counter reached
        smp_server.send_data("STATUS",struct.pack('B', 1))          # SEND STATUS DONE
        Reset_Count += 1 
    else:
        smp_server.send_data("SPEED_SET", struct.pack('2H', 1000, 1000)) 
    
def callback_Mode(payload: bytearray)-> None:
    """ Callback MODE Channel"""
    driving_mode = payload[0]
    if driving_mode:
        print("Driving Mode Selected")
    else:
        print("Train Mode Selected")


m_serialMuxProtChannelIdSPEED_SET = smp_server.create_channel("SPEED_SET", SPEED_SET_DLC)
m_serialMuxProtChannelIdSTATUS = smp_server.create_channel("STATUS", STATUS_DLC)
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

    if m_serialMuxProtChannelIdSPEED_SET == 0 or  m_serialMuxProtChannelIdSTATUS == 0 :
        print("Channel SPEED_SET not created.")
    else:
        if robot_node is None:
            print(f"Robot DEF {ROBOT_NAME} not found.")
            status = -1         
        else:
             while supervisor.step(timestep) != -1:
                   m_elapsedTimeSinceReset += timestep
                   smp_server.process(m_elapsedTimeSinceReset)
                   if Reset_Count != 0:
                       reinitialize()                  
    return status

sys.exit(main_loop())