# Radon Ulzer - Remote Control <!-- omit in toc -->

* [General](#general)
  * [SerialMuxProt Channels](#serialmuxprot-channels)
    * [Rx channel "REMOTE\_CMD"](#rx-channel-remote_cmd)
    * [Tx channel "REMOTE\_RSP"](#tx-channel-remote_rsp)
    * [Rx channel "MOT\_SPEEDS"](#rx-channel-mot_speeds)
    * [Tx channel "LINE\_SENS"](#tx-channel-line_sens)
* [SW Architecture](#sw-architecture)
  * [Logical View](#logical-view)
    * [Application](#application)
      * [Startup](#startup)
      * [LineSensorsCalibration](#linesensorscalibration)
      * [MotorSpeedCalibration](#motorspeedcalibration)
      * [RemoteCtrl](#remotectrl)
      * [Error](#error)
  * [Process View](#process-view)
* [Abbreviations](#abbreviations)
* [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
* [License](#license)
* [Contribution](#contribution)

# General

The robot can be controlled remotely by using the SerialMuxProt protocol.

On target the physical communication uses the serial.

On simulation the physical communication uses a socket connection.

## SerialMuxProt Channels

### Rx channel "REMOTE_CMD"
This channel is used to receive commands, which will be executed by the application in RemoteCtrl state. A command related response will be sent via the "REMOTE_RSP" channel.

Possible commands:
* NO_ACTION (0) - Nothing will happend and no response will be sent.
* START_LINE_SENSOR_CALIB (1) - Start line sensor calibration.
* START_MOTOR_SPEED_CALIB (2) - Start motor speed calibration.
* REINIT_BOARD (3) - Re-initialized the board. Required for webots simulation.

### Tx channel "REMOTE_RSP"
This channel is used to send command related responses. A response will be sent only once and not periodically.

Possible responses:
* OK (0) - Command successful executed.
* PENDING (1) - Command execution is pending.
* ERROR (2) - Command execution failed.

### Rx channel "MOT_SPEEDS"
This channel is used to receive linear motor speed information for the left and right motor in [mm/s].

* The dataypes int16_t is used.
* Order:
  * The left motor speed.
  * The right motor speed.
* Endianess: Big endian

### Tx channel "LINE_SENS"
This channel is used to send line sensor informations in digits.

* The dataypes uint16_t is used.
* Order:
  * From most left to right line sensor.
* Endianess: Big endian

# SW Architecture
The following part contains the specific details of the RemoteControl application.

## Logical View

### Application

![application](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/LogicalView/RemoteControl/Application.plantuml)

#### Startup

![startup](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/LogicalView/RemoteControl/StartupState.plantuml)

#### LineSensorsCalibration

![lineSensorsCalibration](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/LogicalView/RemoteControl/LineSensorsCalibrationState.plantuml)

#### MotorSpeedCalibration

![MotorSpeedCalibration](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/LogicalView/RemoteControl/LineSensorsCalibrationState.plantuml)

#### RemoteCtrl

![driving](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/LogicalView/RemoteControl/RemoteCtrlState.plantuml)

#### Error

![error](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/LogicalView/RemoteControl/ErrorState.plantuml)

## Process View

![processView](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/ProcessView/RemoteControl/SystemStates.plantuml)

# Abbreviations

| Abbreviation | Description |
| - | - |
| HAL | Hardware Abstraction Layer |

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/RadonUlzer/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
