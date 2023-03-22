# Radon Alcer <!-- omit in toc -->

- [SW Architecture](#sw-architecture)
- [Abbreviations](#abbreviations)
  - [Physical View](#physical-view)
  - [Development View](#development-view)
    - [Layers](#layers)
  - [Logical View](#logical-view)
    - [Application](#application)
    - [Service](#service)
      - [Differential Drive](#differential-drive)
      - [Odometry](#odometry)
      - [Speedometer](#speedometer)
    - [HAL](#hal)
  - [Process View](#process-view)
    - [Differential Drive](#differential-drive-1)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

# SW Architecture
The Radon Alcer repository contains several applications. Each application is described according to the 4+1 architectural view.

![architecturalView](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonAlcer/main/doc/architecture/uml/ViewModels.plantuml)

# Abbreviations

| Abbreviation | Description |
| - | - |
| HAL | Hardware Abstraction Layer |

## Physical View
The physical view shows the deployment which is equal to all applications. But not every application may use all of the provided sensors and actors.

![physicalView](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonAlcer/main/doc/architecture/uml/PhysicalView/Deployment.plantuml)

## Development View

### Layers

![deployment](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonAlcer/main/doc/architecture/uml/DevelopmentView/Layers.plantuml)

## Logical View

### Application
The following applications are supported:

* [LineFollower application](LINEFOLLOWER.md)
* [RemoteControl application](REMOTECONTROL.md)

### Service

![service](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonAlcer/main/doc/architecture/uml/LogicalView/Service.plantuml)

#### Differential Drive

![differentialDrive](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonAlcer/main/doc/architecture/uml/LogicalView/DifferentialDrive.plantuml)

#### Odometry

![odometry](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonAlcer/main/doc/architecture/uml/LogicalView/Odometry.plantuml)

Base equations:
* $distanceLeft [mm] = \frac{encoderStepsLeft [steps]}{encoderStepsPerMM [\frac{steps}{mm}]}$
* $distanceRight [mm] = \frac{encoderStepsRight [steps]}{encoderStepsPerMM [\frac{steps}{mm}]}$
* $stepsCenter [steps] = \frac{encoderStepsLeft - encoderStepsRight}{2}$
* $distanceCenter [mm] = \frac{stepsCenter [steps]}{encoderStepsPerMM [\frac{steps}{mm}]}$

Orientation:
* $alpha [rad] = \frac{distanceRight [mm] - distanceLeft [mm]}{wheelBase [mm]}$
* $orientation' [rad] = orientation [rad] + alpha [rad]$

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

![speedometer](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonAlcer/main/doc/architecture/uml/LogicalView/Speedometer.plantuml)

### HAL
The hardware abstraction layer (HAL) for the target is shown. For the simulation and the test only the realization part is different and may base on other 3rd party components.

![hal](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonAlcer/main/doc/architecture/uml/LogicalView/HAL.plantuml)

## Process View

### Differential Drive

![differentialDrive](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonAlcer/main/doc/architecture/uml/ProcessView/DifferentialDrive.plantuml)

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/RadonAlcer/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
