"""Supervisor controller.
    Details: https://github.com/cyberbotics/webots/blob/master/docs/guide/supervisor-programming.md
"""

import sys
import paho.mqtt.client as mqtt
import json
from controller import Supervisor
from controller import AnsiCodes


# Switches
ENABLE = False  # Enable the supervisor
CHECK_COLLISIONS = False  # Check for collisions between vehicles
CHECK_IVS = False  # Check for inter-vehicle space
CHECK_PLATOON_LENGTH = False  # Check for platoon length
DRAW_MARKERS = False  # Draw markers for the vehicles


# The PROTO DEF names must be given!
LEADER_NAME = "LEADER"
FOLLOWER1_NAME = "FOLLOWER1"
FOLLOWER2_NAME = "FOLLOWER2"

# Create the Supervisor instance.
supervisor = Supervisor()
root_node = supervisor.getRoot()
children_field = root_node.getField('children')
leader_node = supervisor.getFromDef(LEADER_NAME)
follower1_node = supervisor.getFromDef(FOLLOWER1_NAME)
follower2_node = supervisor.getFromDef(FOLLOWER2_NAME)
error_detected = False

# Platoon constants
VEHICLE_LENGTH = 100  # mm
NUMBER_OF_VEHICLES = 3  # Number of vehicles in the platoon
MIN_PLATOON_LENGTH = (VEHICLE_LENGTH * NUMBER_OF_VEHICLES)  # mm
MAX_PLATOON_LENGTH = 800  # mm
MIN_INTER_VEHICLE_SPACE = VEHICLE_LENGTH/5  # mm
MAX_INTER_VEHICLE_SPACE = VEHICLE_LENGTH * NUMBER_OF_VEHICLES  # mm
MAX_VEHICLE_SPEED = 3000  # mm/s

# Platoon data
platoon_data = [None] * NUMBER_OF_VEHICLES


def print_constants() -> None:
    """
    Print platoon constants.
    """

    print(f"Vehicle length: {VEHICLE_LENGTH} mm")
    print(f"Number of vehicles: {NUMBER_OF_VEHICLES}")
    print(f"Minimum platoon length: {MIN_PLATOON_LENGTH} mm")
    print(f"Maximum platoon length: {MAX_PLATOON_LENGTH} mm")
    print(f"Minimum inter-vehicle space: {MIN_INTER_VEHICLE_SPACE:.0f} mm")
    print(f"Maximum inter-vehicle space: {MAX_INTER_VEHICLE_SPACE} mm")
    print(f"Maximum vehicle speed: {MAX_VEHICLE_SPEED} mm/s")


def check_crash() -> None:
    """
    Check for collisions between vehicles.
    This is a really slow operation. It should be used only for debugging purposes. It slows the simulation down to 20% of RT. 
    """
    global error_detected

    if not CHECK_COLLISIONS:
        return

    initial_contact_leader = 16
    initial_contact_follower1 = 16
    initial_contact_follower2 = 16

    contact_leader = len(leader_node.getContactPoints(True))
    contact_follower1 = len(follower1_node.getContactPoints(True))
    contact_follower2 = len(follower2_node.getContactPoints(True))

    if (contact_leader > initial_contact_leader) or (contact_follower1 > initial_contact_follower1) or (contact_follower2 > initial_contact_follower2):
        print(AnsiCodes.RED_FOREGROUND +
              "Collision detected" + AnsiCodes.RESET)
        error_detected = True


def process_data() -> None:
    """
    Process data from subscribed topics.

    """
    global error_detected
    distance = [0, 0]
    ivs = [0, 0]

    if error_detected:
        return

    platoon_positions = {"x": [platoon_data[0]["X"], platoon_data[1]["X"], platoon_data[2]["X"]],
                         "y": [platoon_data[0]["Y"], platoon_data[1]["Y"], platoon_data[2]["Y"]]}

    distance[0] = ((platoon_positions["x"][0] - platoon_positions["x"][1])**2 +
                   (platoon_positions["y"][0] - platoon_positions["y"][1])**2)**0.5
    distance[1] = ((platoon_positions["x"][1] - platoon_positions["x"][2])**2 +
                   (platoon_positions["y"][1] - platoon_positions["y"][2])**2)**0.5

    platoon_length = distance[0] + distance[1] + 100

    ivs[0] = distance[0] - 100
    ivs[1] = distance[1] - 100

    if CHECK_PLATOON_LENGTH:
        if platoon_length > MAX_PLATOON_LENGTH:
            print(AnsiCodes.RED_FOREGROUND +
                  f"Platoon length: {platoon_length:.0f} greater than the maximum value {MAX_PLATOON_LENGTH}!" + AnsiCodes.RESET)
            error_detected = True
        elif platoon_length < MIN_PLATOON_LENGTH:
            print(AnsiCodes.RED_FOREGROUND +
                  f"Platoon length: {platoon_length:.0f} smaller than the minimum value {MIN_PLATOON_LENGTH}!" + AnsiCodes.RESET)
            error_detected = True

    if CHECK_IVS:
        for i in range(2):
            if ivs[i] > MAX_INTER_VEHICLE_SPACE:
                print(AnsiCodes.RED_FOREGROUND +
                      f"IVS {i}: {ivs[i]:.0f} greater than the maximum value {MAX_INTER_VEHICLE_SPACE}!" + AnsiCodes.RESET)
                error_detected = True
            # elif ivs[i] < MIN_INTER_VEHICLE_SPACE:
            #     print(AnsiCodes.RED_FOREGROUND +
            #           f"IVS {i}: {ivs[i]:.0f} smaller than the minimum value {MIN_INTER_VEHICLE_SPACE}!" + AnsiCodes.RESET)
            #     error_detected = True

    if CHECK_COLLISIONS:
        check_crash()


def local_topic_callback(client, userdata, message) -> None:
    """
    Callback function for subscribed topics.

    """
    json_payload = json.loads(message.payload)
    array_id = int(json_payload["id"])
    platoon_data[array_id] = json_payload["data"]
    if (not platoon_data.__contains__(None)) and (2 == array_id) and (10000 < int(json_payload["timestamp"])):
        process_data()


def topic_callback(client, userdata, message) -> None:
    if message.topic == "reset":
        print("Resetting")
        supervisor.worldReload()
    elif message.topic == "debug/waypoint":
        local_topic_callback(client, userdata, message)
    elif DRAW_MARKERS:
        jsonPayload = json.loads(message.payload.decode('utf-8'))
        x = (jsonPayload["data"]["X"] / 1000)
        y = (jsonPayload["data"]["Y"] / 1000)

        if message.topic == "platoons/0/vehicles/1/inputWaypoint":
            # print(f"Leader: {x}:{y}")
            children_field.importMFNodeFromString(
                -1, 'Marker {translation ' + str(x) + ' ' + str(y) + ' 0.4 name "" color 0.666667 0 0}')
        elif message.topic == "platoons/0/vehicles/2/inputWaypoint":
            # print(f"Follower 1: {x}:{y}")
            children_field.importMFNodeFromString(
                -1, 'Marker {translation ' + str(x) + ' ' + str(y) + ' 0.4 name "" color 0 0.666667 0}')
        elif message.topic == "platoons/0/vehicles/3/inputWaypoint":
            # print(f"Follower 2: {x}:{y}")
            children_field.importMFNodeFromString(
                -1, 'Marker {translation ' + str(x) + ' ' + str(y) + ' 0.4 name "" color 0 0 0.666667}')


def main_loop():
    """Main loop:
        - Perform simulation steps until Webots is stopping the controller-

    Returns:
        number: Status
    """
    global error_detected
    status = 0

    if not ENABLE:
        print("Platoon Supervisor disabled")
        return status

    supervisor.simulationSetMode(Supervisor.SIMULATION_MODE_REAL_TIME)

    print("Platoon Supervisor started")
    print_constants()

    client = mqtt.Client()
    client.on_message = topic_callback
    client.connect("localhost")
    client.subscribe([("platoons/0/vehicles/1/inputWaypoint", 0),
                     ("platoons/0/vehicles/2/inputWaypoint", 0), ("platoons/0/vehicles/3/inputWaypoint", 0), ("reset", 0), ("debug/waypoint", 0)])
    client.loop_start()

    # Get the time step of the current world.
    timestep = int(supervisor.getBasicTimeStep())

    # leader_node = supervisor.getFromDef(LEADER_NAME)
    # follower1_node = supervisor.getFromDef(FOLLOWER1_NAME)
    # follower2_node = supervisor.getFromDef(FOLLOWER2_NAME)

    if leader_node is None or follower1_node is None or follower2_node is None:
        print(
            f"Robot DEF {LEADER_NAME} or {FOLLOWER1_NAME} or {FOLLOWER2_NAME} not found.")
        status = -1
    else:
        while supervisor.step(timestep) != -1:
            if error_detected:
                leader_node.restartController()
                follower1_node.restartController()
                follower2_node.restartController()
                break

    client.disconnect()
    client.loop_stop()

    return status


sys.exit(main_loop())
