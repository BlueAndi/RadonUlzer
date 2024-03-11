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


def topic_callback(client, userdata, message) -> None:
    if message.topic == "reset":
        print("Resetting")
        supervisor.worldReload()
    else:
        jsonPayload = json.loads(message.payload.decode('utf-8'))
        x = (jsonPayload["data"]["X"] / 1000)
        y = (jsonPayload["data"]["Y"] / 1000)
        print(f"{x}:{y}")

        if message.topic == "platoons/0/vehicles/1/inputWaypoint":
            children_field.importMFNodeFromString(
                -1, 'Marker {translation ' + str(x) + ' ' + str(y) + ' 0 name "" color 0.666667 0 0}')
        elif message.topic == "platoons/0/vehicles/2/inputWaypoint":
            children_field.importMFNodeFromString(
                -1, 'Marker {translation ' + str(x) + ' ' + str(y) + ' 0 name "" color 0 0.666667 0}')
        elif message.topic == "platoons/0/vehicles/3/inputWaypoint":
            children_field.importMFNodeFromString(
                -1, 'Marker {translation ' + str(x) + ' ' + str(y) + ' 0 name "" color 0 0 0.666667}')


def main_loop():
    """Main loop:
        - Perform simulation steps until Webots is stopping the controller-

    Returns:
        number: Status
    """
    status = 0

    client = mqtt.Client()
    client.on_message = topic_callback
    client.connect("localhost")
    client.subscribe([("platoons/0/vehicles/1/inputWaypoint", 0),
                     ("platoons/0/vehicles/2/inputWaypoint", 0), ("platoons/0/vehicles/3/inputWaypoint", 0), ("reset", 0)])
    client.loop_start()

    # Get the time step of the current world.
    timestep = int(supervisor.getBasicTimeStep())

    while supervisor.step(timestep) != -1:
        pass

    client.loop_stop()
    return status


sys.exit(main_loop())
