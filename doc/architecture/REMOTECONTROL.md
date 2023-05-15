# Radon Ulzer - Remote Control <!-- omit in toc -->

* [SW Architecture](#sw-architecture)
* [Abbreviations](#abbreviations)
  * [Logical View](#logical-view)
    * [Application](#application)
      * [Startup](#startup)
      * [LineSensorsCalibration](#linesensorscalibration)
      * [MotorSpeedCalibration](#motorspeedcalibration)
      * [RemoteCtrl](#remotectrl)
      * [Error](#error)
  * [Process View](#process-view)
* [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
* [License](#license)
* [Contribution](#contribution)

# SW Architecture
The following part contains the specific details of the RemoteControl application.

# Abbreviations

| Abbreviation | Description |
| - | - |
| HAL | Hardware Abstraction Layer |

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

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/RadonUlzer/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
