# Radon Ulzer - SENSORFUSION <!-- omit in toc -->

* [General](#general)
  * [SerialMuxProt Channels](#serialmuxprot-channels)
    * [Tx channel "SENSOR\_DATA"](#tx-channel-sensor_data)
    * [Tx channel "END\_LINE"](#tx-channel-end_line)
* [SW Architecture](#sw-architecture)
  * [Logical View](#logical-view)
    * [Application](#application)
    * [HAL](#hal)
  * [Process View](#process-view)
* [Abbreviations](#abbreviations)
* [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
* [License](#license)
* [Contribution](#contribution)

# General

The robot sends sensor data (Odometry data and IMU data) via SerialMuxProt. 

On target the physical communication uses the serial.

On simulation the physical communication uses a socket connection.

## SerialMuxProt Channels
All channels are defined in the file [SerialMuxChannels.h](https://github.com/BlueAndi/RadonUlzer/blob/main/lib/APPSensorFusion/SerialMuxChannels.h)

### Tx channel "SENSOR_DATA"
This channel is used to send raw sensor data used for Sensor Fusion on the ZumoComSystem.

* The datatypes can be found in SerialMuxChannel.h.
* Order:
  * Acceleration in X (raw sensor value in digits)
  * Acceleration in Y (raw sensor value in digits)
  * turnRateZ around Z (raw sensor value in digits)
  * Magnetometer value in X (raw sensor value in digits)
  * Magnetometer value in Y (raw sensor value in digits)
  * Angle calculated by Odometry (in mrad)
  * Position in X calculated by Odometry (in mm)
  * Position in Y calculated by Odometry (in mm)
  * Time passed since the last sensor value (in ms)
* Endianess: Big endian

### Tx channel "END_LINE"
This channel is used to send a flag to signal that a new End Line has been detected. This is used for testing purposes. 
The only content that is sent is a boolean variable with the value true. 

# SW Architecture
The following part contains the specific details of the SensorFusion application.

## Logical View

### Application
The application uses the same [State Machine](https://github.com/BlueAndi/RadonUlzer/blob/main/doc/architecture/LINEFOLLOWER.md) as the Line Follower Application.

### HAL
Some changes have been made to the HAL compared to the HAL of the other Applications.
ButtonB, ButtonC, LedYellow, LedRed, the Display and the ProximitySensors have been removed in the App specific HAL. 
![HALSensorFusion](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/LogicalView/SensorFusion/HAL_SensorFusion.puml)

An IMU has been added:
![HALIMU](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/LogicalView/SensorFusion/HAL_IMU.puml)

## Process View
Compared to the System States of the [Linefollower Application](https://github.com/BlueAndi/RadonUlzer/blob/main/doc/architecture/LINEFOLLOWER.md), the only change that has been made is that the detection of an end line does not trigger a change of state from DrivingState to ReadyState. This is done for testing purposes.  
![processView](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/ProcessView/SensorFusion/SystemStates.puml)

# Abbreviations

| Abbreviation | Description |
| - | - |
| HAL | Hardware Abstraction Layer |
| IMU | Inertial Measurement Unit |

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/RadonUlzer/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
