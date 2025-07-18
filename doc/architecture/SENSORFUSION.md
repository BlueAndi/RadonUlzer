# Radon Ulzer - SENSORFUSION <!-- omit in toc -->

- [General](#general)
  - [SerialMuxProt Channels](#serialmuxprot-channels)
    - [Tx channel "SENSOR\_DATA"](#tx-channel-sensor_data)
- [SW Architecture](#sw-architecture)
  - [Logical View](#logical-view)
  - [Process View](#process-view)
- [Abbreviations](#abbreviations)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

## General

The robot sends sensor data (Odometry data and IMU data) via SerialMuxProt.

On target the physical communication uses the serial.

On simulation the physical communication uses a socket connection.

### SerialMuxProt Channels

All channels are defined in the file [SerialMuxChannels.h](https://github.com/BlueAndi/RadonUlzer/blob/main/lib/APPSensorFusion/SerialMuxChannels.h)

#### Tx channel "SENSOR_DATA"

This channel is used to send raw sensor data used for Sensor Fusion on the ZumoComSystem.

- The datatypes can be found in SerialMuxChannel.h.
- Order:
  - Position in X calculated by Odometry (in mm)
  - Position in Y calculated by Odometry (in mm)
  - Orientation calculated by Odometry (in mrad)
  - Acceleration in X (raw sensor value in digits)
  - Turnrate around Z (raw sensor value in digits)
  - Time passed since the last sensor value (in ms)
- Endianess: Big endian

## SW Architecture

The following part contains the specific details of the SensorFusion application.

### Logical View

The States are similar to the [Linefollower Application](https://github.com/BlueAndi/RadonUlzer/blob/main/doc/architecture/LINEFOLLOWER.md)

### Process View

![processView](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/ProcessView/SensorFusion/SystemStates.puml)

## Abbreviations

| Abbreviation | Description |
| - | - |
| HAL | Hardware Abstraction Layer |
| IMU | Inertial Measurement Unit |

## Issues, Ideas And Bugs

If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/RadonUlzer/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

## License

The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

## Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
