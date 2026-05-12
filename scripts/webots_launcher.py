"""Webots launcher"""

# MIT License
#
# Copyright (c) 2022 - 2026 Andreas Merkle (web@blue-andi.de)
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
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

################################################################################
# Imports
################################################################################
import os
import sys
import platform

# pyright: reportUndefinedVariable=false
Import("env") # pylint: disable=undefined-variable

################################################################################
# Variables
################################################################################
OS_PLATFORM_TYPE_WIN = "Windows"
OS_PLATFORM_TYPE_LINUX = "Linux"
OS_PLATFORM_TYPE_MACOS = "Darwin"
OS_PLATFORM_TYPE = platform.system()

WEBOTS_IP_ADDRESS = env.GetProjectOption("custom_webots_ip_address")  # pylint: disable=undefined-variable
WEBOTS_PROTOCOL = env.GetProjectOption("custom_webots_protocol")  # pylint: disable=undefined-variable

ROBOT_NAME = env.GetProjectOption("custom_webots_robot_name") # pylint: disable=undefined-variable

SUPERVISOR_SERIAL_RX_CHANNEL = env.GetProjectOption("custom_webots_supervisor_serial_rx_channel") # pylint: disable=undefined-variable
SUPERVISOR_SERIAL_TX_CHANNEL = env.GetProjectOption("custom_webots_supervisor_serial_tx_channel") # pylint: disable=undefined-variable

ROBOT_SERIAL_RX_CHANNEL = env.GetProjectOption("custom_webots_robot_serial_rx_channel") # pylint: disable=undefined-variable
ROBOT_SERIAL_TX_CHANNEL = env.GetProjectOption("custom_webots_robot_serial_tx_channel") # pylint: disable=undefined-variable

SETTINGS_PATH = env.GetProjectOption("custom_settings_path") # pylint: disable=undefined-variable

PROJECT_PATH = env["PROJECT_DIR"]  # pylint: disable=undefined-variable
ABS_SETTINGS_PATH = os.path.join(PROJECT_PATH, SETTINGS_PATH)
PROGRAM_PATH = "$BUILD_DIR/"
PROGRAM_OPTIONS = '--settingsPath "' \
                + ABS_SETTINGS_PATH + '" ' \
                + '--supervisorRxCh ' + SUPERVISOR_SERIAL_RX_CHANNEL + ' ' \
                + '--supervisorTxCh ' + SUPERVISOR_SERIAL_TX_CHANNEL + ' '
PROGRAM_OPTIONS_ZUMO_COM_SYSTEM = '-c ' \
                                + '--supervisorRxCh ' + SUPERVISOR_SERIAL_RX_CHANNEL + ' ' \
                                + '--supervisorTxCh ' + SUPERVISOR_SERIAL_TX_CHANNEL + ' ' \
                                + '--serialRxCh ' + ROBOT_SERIAL_RX_CHANNEL + ' ' \
                                + '--serialTxCh ' + ROBOT_SERIAL_TX_CHANNEL + ' ' \
                                + '--settingsPath "' + ABS_SETTINGS_PATH + '" ' \
                                + '-v'
WEBOTS_CONTROLLER_OPTIONS = '--robot-name=' + ROBOT_NAME + ' ' \
                            '--stdout-redirect' + ' ' \
                            '--ip-address=' + WEBOTS_IP_ADDRESS + ' ' \
                            '--protocol=' + WEBOTS_PROTOCOL
WEBOTS_HOME = os.getenv('WEBOTS_HOME')

if WEBOTS_HOME is None:
    print("WEBOTS_HOME environment variable is not set. "
          "Please set it to the Webots installation directory.")
    sys.exit(1)

if OS_PLATFORM_TYPE == OS_PLATFORM_TYPE_WIN:

    WEBOTS_HOME = WEBOTS_HOME.replace('\\', '/')
    WEBOTS_CONTROLLER  = "\""
    WEBOTS_CONTROLLER += f"{WEBOTS_HOME}/msys64/mingw64/bin/webots-controller.exe"
    WEBOTS_CONTROLLER += "\""
    PROGRAM_NAME = "${PROGNAME}.exe"

elif OS_PLATFORM_TYPE == OS_PLATFORM_TYPE_LINUX:

    WEBOTS_CONTROLLER = f"{WEBOTS_HOME}/webots-controller"
    PROGRAM_NAME = "${PROGNAME}"

elif OS_PLATFORM_TYPE == OS_PLATFORM_TYPE_MACOS:

    WEBOTS_CONTROLLER = f"{WEBOTS_HOME}/Contents/MacOS/webots-controller"
    PROGRAM_NAME = "${PROGNAME}"

else:
    print(f"OS type {OS_PLATFORM_TYPE} not supported.")
    sys.exit(1)

WEBOTS_LAUNCHER_ACTION = WEBOTS_CONTROLLER + ' '\
                        + WEBOTS_CONTROLLER_OPTIONS + ' ' \
                        + PROGRAM_PATH + PROGRAM_NAME + ' ' \
                        + PROGRAM_OPTIONS

WEBOTS_LAUNCHER_ZUMO_COM_SYSTEM_ACTION = WEBOTS_CONTROLLER + ' ' \
                                        + WEBOTS_CONTROLLER_OPTIONS + ' ' \
                                        + PROGRAM_PATH + PROGRAM_NAME + ' ' \
                                        + PROGRAM_OPTIONS_ZUMO_COM_SYSTEM

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

def _check_extern_slot(protocol, ip, robot_name):
    """Return True if the named robot has a free extern controller slot in Webots."""
    import getpass
    import socket

    if protocol == "ipc":
        ipc_socket = f"/tmp/webots/{getpass.getuser()}/1234/ipc/{robot_name}/extern"
        if not os.path.exists(ipc_socket):
            return False
        # The socket file exists even when already claimed — try connecting to confirm it's free.
        try:
            s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            s.settimeout(1)
            s.connect(ipc_socket)
            s.close()
            return True
        except OSError:
            return False
    elif protocol == "tcp":
        try:
            s = socket.create_connection((ip, 1234), timeout=2)
            s.close()
            # TCP only confirms Webots is reachable; robot slot presence cannot be verified remotely.
            print(f"Warning: Cannot verify extern slot for '{robot_name}' over TCP — proceeding.")
            return True
        except OSError as exc:
            raise SystemExit(1) from exc
    return False

def _get_available_ipc_slots():
    """Return list of robot names with extern slots currently registered via IPC."""
    import getpass
    ipc_dir = f"/tmp/webots/{getpass.getuser()}/1234/ipc"
    if not os.path.isdir(ipc_dir):
        return None  # Webots not running
    return [name for name in os.listdir(ipc_dir)
            if os.path.exists(os.path.join(ipc_dir, name, "extern"))]

def _abort_missing_slot(protocol, ip, *robot_names):
    """Check all robot names have extern slots, print a clear error and exit if not."""
    import socket

    if protocol == "tcp":
        try:
            s = socket.create_connection((ip, 1234), timeout=2)
            s.close()
        except OSError:
            print(f"Error: Cannot reach Webots at {ip}:1234 via TCP. Is Webots running?")
            raise SystemExit(1)
        print(f"Warning: Cannot verify extern slots over TCP — make sure the correct world is loaded.")
        return

    available = _get_available_ipc_slots()
    if available is None:
        print("Error: Webots is not running or no world is loaded.")
        raise SystemExit(1)
    missing = [n for n in robot_names if not _check_extern_slot(protocol, ip, n)]
    if missing:
        for name in missing:
            print(f"Error: Robot '{name}' has no extern controller slot in the loaded world.")
        slots = ", ".join(available) if available else "(none)"
        print(f"       Available extern slots: {slots}")
        print("       Load a world where the missing robot(s) have controller \"<extern>\".")
        raise SystemExit(1)

def check_webots_ready(source, target, env):  # pylint: disable=unused-argument
    """Check that Webots is running and the robot has an extern controller slot."""
    protocol = env.GetProjectOption("custom_webots_protocol")
    ip = env.GetProjectOption("custom_webots_ip_address")
    robot_name = env.GetProjectOption("custom_webots_robot_name")
    _abort_missing_slot(protocol, ip, robot_name)

def check_webots_ready_zumo_com_system(source, target, env):  # pylint: disable=unused-argument
    """Check extern slots for both the Zumo robot and the ZumoComSystem robot."""
    protocol = env.GetProjectOption("custom_webots_protocol")
    ip = env.GetProjectOption("custom_webots_ip_address")
    robot_name = env.GetProjectOption("custom_webots_robot_name")
    _abort_missing_slot(protocol, ip, robot_name, "ZumoComSystem")

################################################################################
# Main
################################################################################

# pylint: disable=undefined-variable
env.AddCustomTarget(
    name="webots_launcher",
    dependencies=PROGRAM_PATH + PROGRAM_NAME,
    actions=[
        check_webots_ready,
        WEBOTS_LAUNCHER_ACTION
    ],
    title="Launch alone",
    description="Launch application with Webots launcher and without ZumoComSystem."
)

# pylint: disable=undefined-variable
env.AddCustomTarget(
    name="webots_launcher_zumo_com_system",
    dependencies=PROGRAM_PATH + PROGRAM_NAME,
    actions=[
        check_webots_ready_zumo_com_system,
        WEBOTS_LAUNCHER_ZUMO_COM_SYSTEM_ACTION
    ],
    title="Launch with ZumoComSystem",
    description="Launch application with Webots launcher and with ZumoComSystem."
)
