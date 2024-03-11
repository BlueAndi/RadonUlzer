"""Supervisor controller.
    Details: https://github.com/cyberbotics/webots/blob/master/docs/guide/supervisor-programming.md
"""

import sys
import paho.mqtt.client as mqtt
import json
from controller import Supervisor

# Create the Supervisor instance.
supervisor = Supervisor()
root_node = supervisor.getRoot()
children_field = root_node.getField('children')

# The PROTO DEF names must be given!
LEADER_NAME = "LEADER"
FOLLOWER1_NAME = "FOLLOWER1"
FOLLOWER2_NAME = "FOLLOWER2"


def topic_callback(client, userdata, message) -> None:
    if message.topic == "reset":
        print("Resetting")
        supervisor.worldReload()
    else:
        jsonPayload = json.loads(message.payload.decode('utf-8'))
        x = (jsonPayload["data"]["X"] / 1000)
        y = (jsonPayload["data"]["Y"] / 1000)

        if message.topic == "platoons/0/vehicles/1/inputWaypoint":
            print(f"Leader: {x}:{y}")
            children_field.importMFNodeFromString(
                -1, 'Marker {translation ' + str(x) + ' ' + str(y) + ' 0 name "" color 0.666667 0 0}')
        elif message.topic == "platoons/0/vehicles/2/inputWaypoint":
            print(f"Follower 1: {x}:{y}")
            children_field.importMFNodeFromString(
                -1, 'Marker {translation ' + str(x) + ' ' + str(y) + ' 0 name "" color 0 0.666667 0}')
        elif message.topic == "platoons/0/vehicles/3/inputWaypoint":
            print(f"Follower 2: {x}:{y}")
            children_field.importMFNodeFromString(
                -1, 'Marker {translation ' + str(x) + ' ' + str(y) + ' 0 name "" color 0 0 0.666667}')


def main_loop():
    """Main loop:
        - Perform simulation steps until Webots is stopping the controller-

    Returns:
        number: Status
    """
    status = 0
    supervisor.simulationSetMode(Supervisor.SIMULATION_MODE_REAL_TIME)

    client = mqtt.Client()
    client.on_message = topic_callback
    client.connect("localhost")
    client.subscribe([("platoons/0/vehicles/1/inputWaypoint", 0),
                     ("platoons/0/vehicles/2/inputWaypoint", 0), ("platoons/0/vehicles/3/inputWaypoint", 0), ("reset", 0)])
    client.loop_start()

    # Get the time step of the current world.
    timestep = int(supervisor.getBasicTimeStep())

    leader_node = supervisor.getFromDef(LEADER_NAME)
    follower1_node = supervisor.getFromDef(FOLLOWER1_NAME)
    follower2_node = supervisor.getFromDef(FOLLOWER2_NAME)

    if leader_node is None or follower1_node is None or follower2_node is None:
        print(
            f"Robot DEF {LEADER_NAME} or {FOLLOWER1_NAME} or {FOLLOWER2_NAME} not found.")
        status = -1
    else:

        initial_contact_leader = 16
        initial_contact_follower1 = 16
        initial_contact_follower2 = 16

        while supervisor.step(timestep) != -1:
            contact_leader = len(leader_node.getContactPoints(True))
            contact_follower1 = len(follower1_node.getContactPoints(True))
            contact_follower2 = len(follower2_node.getContactPoints(True))

            if (contact_leader > initial_contact_leader) or (contact_follower1 > initial_contact_follower1) or (contact_follower2 > initial_contact_follower2):
                print("Collision detected")
                supervisor.worldReload()
                client.disconnect()
                client.loop_stop()
                break

    return status


sys.exit(main_loop())
