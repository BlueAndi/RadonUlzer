# Radon Ulzer - SENSORFUSION <!-- omit in toc -->

* [General](#general)
  * [SerialMuxProt Channels](#serialmuxprot-channels)
    * [Tx channel "LINE\_SENS"](#tx-channel-sensor_data)
* [SW Architecture](#sw-architecture)
  * [Logical View](#logical-view)
    * [Application](#application)
* [Abbreviations](#abbreviations)
* [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
* [License](#license)
* [Contribution](#contribution)

# General

The robot sends sensor data (Odometry data and IMU data) via SerialMuxProt. 

On target the physical communication uses the serial.

On simulation the physical communication uses a socket connection.

## SerialMuxProt Channels


### Tx channel "SENSOR_DATA"
This channel is used to send raw sensor data used for Sensor Fusion on the ZumoComSystem.

* The datatypes can be found in SerialMuxChannel.h.
* Order:
  * Acceleration in X (raw sensor value)
  * Acceleration in Y (raw sensor value)
  * turnRateZ around Z (raw sensor value)
  * Magnetometer value in X (raw sensor value)
  * Magnetometer value in Y (raw sensor value)
  * Angle calculated by Odometry (in mrad)
  * Position in X calculated by Odometry (in mm)
  * Position in Y calculated by Odometry (in mm)
* Endianess: Big endian

# SW Architecture
The following part contains the specific details of the SensorFusion application.

## Logical View

### Application
The application uses the same [State Machine](https://github.com/BlueAndi/RadonUlzer/blob/main/doc/architecture/LINEFOLLOWER.md) as the Line Follower Application.

### HAL
Some changes have been made to the HAL.
![HALSensorFusion](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/SensorFusion/doc/architecture/uml/LogicalView/SensorFusion/HAL_SensorFusion.puml)

ButtonB, ButtonC, the ProximitySensor and the Buzzer have been removed. 
An IMU has been added:
![HALIMU](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/SensorFusion/doc/architecture/uml/LogicalView/SensorFusion/HAL_IMU.puml)

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
