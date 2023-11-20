# Radon Ulzer - Line Follower <!-- omit in toc -->

- [Configuration Management](#configuration-management)
- [ArduinoNative](#arduinonative)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

# Configuration Management

![config](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/main/doc/configuration/uml/configuration.plantuml)

# PlatformIO Environments
As seen in the diagram before, the working environments are defined in the platformio.ini file, and these are used by PlatformIO to build the respective applications. To build an specific application, there are three possible methods:

- On the left task bar of VS Code, choose the PlatformIO extension symbol, and open de drop-down menu of the desired application.
- Choose the desired application on the bottom task bar using the "Switch PlatformIO Project Environment" button.
- Use the command line: ```pio run -e <APPLICATION-NAME>```

# ArduinoNative
In order to maintain compatibility between target and simulation, some interfaces from the Arduino Core have been adapted and/or stubbed into the ArduinoNative library. Code stubs for Stream, Serial, millis(), delay() and other similar Arduino functionalities have been implemented here.

It is important to note that in the case of the simulation, the main entry point of the program is found in Arduino.cpp.

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/RadonUlzer/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
