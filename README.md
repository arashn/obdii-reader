# OBD-II Reader
This is an OBD-II diagnostic system for cars. It reads diagnostic information from a car's OBDII port. It is based on the ATmega32 microcontroller.

For more details about this project, see [this wiki page](https://github.com/arashn/obdii-reader/wiki).

This system does not work with vehicles manufactured prior to 1996, as most pre-1996 vehicles do not have the OBD-II system. Additionally, at the moment, this system only works with vehicles that support the ISO 9141-2 protocol. To determine whether your car supports the ISO 9141-2 protocol, refer to [this webpage](http://www.obdii.com/connector.html).

### Getting Started
To get started with this project, you will need the parts listed in [this page](https://github.com/arashn/obdii-reader/wiki/Required-Parts). You will also need a PC running Windows 7 or higher to load the OBD-II reader program onto the ATmega32 microcontoller.

To set up the system, follow the steps below:

1. Download and install Atmel Studio. You can download Atmel Studio 7 from [here](http://www.atmel.com/tools/ATMELSTUDIO.aspx).
2. Connect the components to the microcontroller as shown in this schematic.
3. Connect the Atmel ICE Basic programmer to the microcontroller, using this diagram to identify the pins on the microcontroller which correspond to the pins on the connector.
4. Connect the programmer to your computer. Wait for the drivers to intall.

### Contributing
You can contribute to this project by helping add support for other protocols, and finding bugs. You may fork this project and modify it as needed.
