# Radon Ulzer <!-- omit in toc -->

* [SW Architecture](#sw-architecture)
* [Abbreviations](#abbreviations)
  * [Physical View](#physical-view)
  * [Development View](#development-view)
    * [Layers](#layers)
  * [Logical View](#logical-view)
    * [Application](#application)
    * [Service](#service)
      * [Differential Drive](#differential-drive)
      * [Odometry](#odometry)
      * [Speedometer](#speedometer)
    * [HAL](#hal)
  * [Process View](#process-view)
    * [Differential Drive](#differential-drive-1)
* [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
* [License](#license)
* [Contribution](#contribution)

# SW Architecture
The Radon Ulzer repository contains several applications. Each application is described according to the 4+1 architectural view.

![architecturalView](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/ViewModels.plantuml)

# Abbreviations

| Abbreviation | Description |
| - | - |
| HAL | Hardware Abstraction Layer |

## Physical View
The physical view shows the deployment which is equal to all applications. But not every application may use all of the provided sensors and actors.

![physicalView](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/PhysicalView/Deployment.plantuml)

## Development View

### Layers

![deployment](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/DevelopmentView/Layers.plantuml)

## Logical View

### Application
The following applications are supported:

* [ConvoyLeader application](CONVOYLEADER.md)
* [LineFollower application](LINEFOLLOWER.md)
* [RemoteControl application](REMOTECONTROL.md)
* [SensorFusion application](SENSORFUSION.md)

### Service

![service](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/LogicalView/Service.plantuml)

#### Differential Drive

![differentialDrive](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/LogicalView/DifferentialDrive.plantuml)

Robot angular speed vs wheel linear speed\
$v [\frac{mm}{s}] = \frac{w_r [\frac{rad}{s}] \cdot W [mm]}{2}$

* $v$: The linear speed of one wheel relative to the other $[\frac{mm}{s}]$.
* $w_r$: The robot's angular speed $[\frac{rad}{s}]$.
* $W$: The robot's wheel base $[mm]$.
* $v_L = -v_R$ by rotation about midpoint of the wheel axis.

Linear speed left\
$v_L [\frac{mm}{s}] = -\frac{w_r [\frac{rad}{s}] \cdot W [mm]}{2}$

Linear speed right\
$v_R [\frac{mm}{s}] = \frac{w_r [\frac{rad}{s}] \cdot W [mm]}{2}$

Consider encoder steps per m\
$v [\frac{steps}{s}] = \frac{w_r [\frac{rad}{s}] \cdot W [mm]}{2} \cdot \frac{ENC [\frac{steps}{m}]}{1000}$

* $v$: The linear speed of one wheel relative to the other $[\frac{steps}{s}]$.
* $w_r$: The robot's angular speed $[\frac{rad}{s}]$.
* $W$: The robot's wheel base $[mm]$.
* $ENC$: The number of encoder steps per m $[\frac{steps}{m}]$.

Linear speed left\
$v_L [\frac{steps}{s}] = -\frac{w_r [\frac{rad}{s}] \cdot W [mm]}{2} \cdot \frac{ENC [\frac{steps}{m}]}{1000}$

Linear speed right\
$v_R [\frac{steps}{s}] = \frac{w_r [\frac{rad}{s}] \cdot W [mm]}{2} \cdot \frac{ENC [\frac{steps}{m}]}{1000}$

Consider robot linear speed center\

Linear speed left\
$v_L [\frac{steps}{s}] = \frac{v_{Linear}}{2} [\frac{steps}{s}] -\frac{w_r [\frac{rad}{s}] \cdot W [mm]}{2} \cdot \frac{ENC [\frac{steps}{m}]}{1000}$

Linear speed right\
$v_R [\frac{steps}{s}] = \frac{v_{Linear}}{2} [\frac{steps}{s}] + \frac{w_r [\frac{rad}{s}] \cdot W [mm]}{2} \cdot \frac{ENC [\frac{steps}{m}]}{1000}$

Consider angular speed in mrad per s\
Linear speed left\
$v_L [\frac{steps}{s}] = \frac{v_{Linear}}{2} [\frac{steps}{s}] -\frac{w_r [\frac{mrad}{s}] \cdot W [mm]}{2 \cdot 1000} \cdot \frac{ENC [\frac{steps}{m}]}{1000}$

Linear speed right\
$v_R [\frac{steps}{s}] = \frac{v_{Linear}}{2} [\frac{steps}{s}] + \frac{w_r [\frac{mrad}{s}] \cdot W [mm]}{2 \cdot 1000} \cdot \frac{ENC [\frac{steps}{m}]}{1000}$

#### Odometry

![odometry](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/LogicalView/Odometry.plantuml)

Base equations:
* $distanceLeft [mm] = \frac{encoderStepsLeft [steps]}{encoderStepsPerMM [\frac{steps}{mm}]}$
* $distanceRight [mm] = \frac{encoderStepsRight [steps]}{encoderStepsPerMM [\frac{steps}{mm}]}$
* $stepsCenter [steps] = \frac{encoderStepsLeft - encoderStepsRight}{2}$
* $distanceCenter [mm] = \frac{stepsCenter [steps]}{encoderStepsPerMM [\frac{steps}{mm}]}$

Orientation:
* $alpha [rad] = \frac{distanceRight [mm] - distanceLeft [mm]}{wheelBase [mm]}$
* $orientation' [rad] = orientation [rad] + alpha [rad]$
* $orientation' [rad] = orientation [rad]~\%~2\pi$
* $-2\pi < Orientation < 2\pi$
* After wrapping on the positive limit $2\pi$, the orientation remains positive and starts from 0 again.
* After wrapping on the negative limit $2\pi$, the orientation remains negative and starts from 0 again.

Position:
* $dX [mm] = -distanceCenter [mm] \cdot sin(orientation' [rad])$ <- Approximation for performance reason
* $dY [mm] = distanceCenter [mm] \cdot cos(orientation' [rad])$ <- Approximation for performance reason
* $x' [mm] = x [mm] + dX [mm]$
* $y' [mm] = y [mm] + dY [mm]$

Improvement for better accuracy:
* $alpha [mrad] = \frac{1000 \cdot (encoderStepsRight [steps] - encoderStepsLeft [steps])}{encoderStepsPerMM [\frac{steps}{mm}] \cdot wheelBase [mm]}$
* $orientation' [mrad] = orientation [mrad] + alpha [mrad]$
* $dX [mm] = -distanceCenter [mm] \cdot sin(\frac{orientation' [mrad]}{1000})$
* $dY [mm] = distanceCenter [mm] \cdot cos(\frac{orientation' [mrad]}{1000})$

#### Speedometer

![speedometer](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/LogicalView/Speedometer.plantuml)

### HAL
The hardware abstraction layer (HAL) depends on the target.
* **ATmega32u4 on the Zumo32u4**: [ZumoHALATmega32u4](https://github.com/BlueAndi/ZumoHALATmega32u4)
* **Zumo in Webots**: [ZumoHALWebots](https://github.com/BlueAndi/ZumoHALWebots)
* **Zumo Test**: [HALTest](/lib/HALTest/)

## Process View

### Differential Drive

![differentialDrive](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/architecture/uml/ProcessView/DifferentialDrive.plantuml)

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/RadonUlzer/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
