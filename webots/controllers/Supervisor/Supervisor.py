"""Supervisor controller.
    Details: https://github.com/cyberbotics/webots/blob/master/docs/guide/supervisor-programming.md
"""
import sys
from Serial_webots import SerialWebots
from SerialMuxProt import SerialMuxProt
from controller import Supervisor


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

def callback_Status(payload: bytearray) -> None:
    """ Callback Status Channel """
    print(payload)

def main_loop():
    """Main loop:
        - Perform simulation steps until Webots is stopping the controller-

    Returns:
        number: Status
    """
    status = 0
    m_elapsedTimeSinceReset = 0
    # Subscribe To Status Channel
    smp_server.subscribe_to_channel("STATUS", callback_Status)
     
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
