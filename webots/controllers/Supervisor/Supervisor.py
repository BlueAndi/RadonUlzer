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

m_serialMuxProtChannelIdSPEED_SET = smp_server.create_channel("SPEED_SET", 4)

def callback_Status(payload: bytearray) -> int:
    """ Callback Status Channel """
    if payload[0] == 1:
       print("End-Start-Line Detected")
       smp_server.send_data("SPEED_SET", struct.pack('2H', 0, 0))  # Stop motors


def callback_LineSensors(payload: bytearray)-> None:
    """ Callback LINE_SENS Channel"""
    sensor_data = struct.unpack('5H', payload)
    for idx in range (5):
        print(f"Sensor[{idx}] = {sensor_data[idx]}")
    smp_server.send_data("SPEED_SET", struct.pack('2H', 1000, 1000))  # Stop motors

def callback_Mode(payload: bytearray)-> None:
    """ Callback MODE Channel"""
    train_mode = payload[0]
    if train_mode:
        print("Train Mode Selected")
    else:
        print("Driving Mode Selected")    

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
        print(f"Channel SPEED_SET not created.")
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
