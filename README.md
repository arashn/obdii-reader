# OBD-II Reader
This is an OBD-II diagnostic system for cars. It reads diagnostic information from a car's OBD-II port. It is based on the ATmega32 microcontroller.

For more details about this project, see [this wiki page](https://github.com/arashn/obdii-reader/wiki).

This system does not work with vehicles manufactured prior to 1996, as most pre-1996 vehicles do not have the OBD-II system. Additionally, at the moment, this system only works with vehicles that support the ISO 9141-2 protocol. To determine whether your car supports the ISO 9141-2 protocol, refer to [this webpage](http://www.obdii.com/connector.html).

### Getting Started
To get started with this project, you will need the parts listed in [this page](https://github.com/arashn/obdii-reader/wiki/Required-Parts). You will also need a PC running Windows 7 or higher to load the OBD-II reader program onto the ATmega32 microcontoller.

To set up the system, follow the steps below:

1. Download and install Atmel Studio. You can download Atmel Studio 7 from [here](http://www.atmel.com/tools/ATMELSTUDIO.aspx).
2. Connect the components to the microcontroller as shown in [this schematic](https://github.com/arashn/obdii-reader/blob/master/diagrams/schematic.png). This picture shows how I connected the components on the breadboard. **Note:** You will need to solder wires to pins 1 through 14 on the LCD.
3. Connect the Atmel ICE Basic programmer to the microcontroller, using [this diagram](https://github.com/arashn/obdii-reader/blob/master/diagrams/connector.png) to identify the pins on the microcontroller which correspond to the pins on the connector.
4. Connect the a 9V battery to the location labeled '9V' on the schematic.
5. Connect the programmer to your computer. Wait for the drivers to install.
6. Launch Atmel Studio. In the main window, click 'Tools', then 'Device Programming'.
7. In the window that opens, for 'Tool', select 'Atmel-ICE'; for 'Device', select 'ATmega32'; and for 'Interface', select 'ISP'. Then click 'Apply'.
8. For 'ISP Clock', a value of 125 kHz is sufficient. Click 'Set'. Then, under 'Device signature', click 'Read'. If everything is set up correctly, the box should read a hexadecimal value. Click 'Close'.
9. Back in the main window, click 'File', then 'New' -> 'Project...'. For the project type, choose 'GCC C ASF Board Project'. Give your project a name, then click 'OK'.
10. In the board selection window, choose 'ATmega32' in the top section, and in the section below, choose 'User Board template - megaAVR'. Click 'Ok'.

### Contributing
You can contribute to this project by helping add support for other protocols, and finding bugs. Please use the issue tracker to see current issues and file new bugs, or to request new features. You may fork this project and make your own additions or modifications to the system.
