# OBD-II Reader
This is an OBD-II diagnostic system for cars. It reads diagnostic information from a car's OBD-II port. It is based on the ATmega32 microcontroller.

For more details about this project, see [this wiki page](https://github.com/arashn/obdii-reader/wiki).

This system does not work with vehicles manufactured prior to 1996, as most pre-1996 vehicles do not have the OBD-II system. Additionally, at the moment, this system only works with vehicles that support the ISO 9141-2 protocol. To determine whether your car supports the ISO 9141-2 protocol, refer to [this webpage](http://www.obdii.com/connector.html).

### Getting Started
To get started with this project, you will need the parts listed in [this page](https://github.com/arashn/obdii-reader/wiki/Required-Parts). You will also need a PC running Windows 7 or higher to load the OBD-II reader program onto the ATmega32 microcontoller.

To set up the system, follow the steps below:

1. Download and install Atmel Studio. You can download Atmel Studio 7 from [here](http://www.atmel.com/tools/ATMELSTUDIO.aspx).
2. Connect the components to the microcontroller as shown in [this schematic](https://github.com/arashn/obdii-reader/blob/master/diagrams/schematic.png). This picture shows how I connected the components on the breadboard. **Note:** You will need to solder wires to pins 1 through 14 on the LCD.
3. Connect the Atmel ICE Basic programmer to the microcontroller, using [this diagram](https://github.com/arashn/obdii-reader/blob/master/diagrams/programmer_connector.png) to identify the pins on the microcontroller which correspond to the pins on the connector.
4. Connect a 9V battery to the location labeled '9V' on the schematic.
5. Connect the programmer to your computer. Wait for the drivers to install.
6. Launch Atmel Studio. In the main window, click 'Tools', then 'Device Programming'.
7. In the window that opens, for 'Tool', select 'Atmel-ICE'; for 'Device', select 'ATmega32'; and for 'Interface', select 'ISP'. Then click 'Apply'.
8. For 'ISP Clock', a value of 125 kHz is sufficient. Click 'Set'. Then, under 'Device signature', click 'Read'. If everything is set up correctly, the box should read a hexadecimal value. Click 'Close'.
9. Back in the main window, click 'File', then 'New' -> 'Project...'. For the project type, choose 'GCC C ASF Board Project'. Give your project a name, then click 'OK'.
10. In the board selection window, choose 'ATmega32' in the top section, and in the section below, choose 'User Board template - megaAVR'. Click 'Ok'.
11. In the Solution Explorer window to the right, double-click the 'src' folder. Then, delete the default main.c file by right-clicking on the file, choosing 'Remove' in the context menu, and clicking 'Delete'.
12. Right-click again on the 'src' folder, then click 'Add' -> 'Existing Item...'. Then, in the window that opens, navigate to the directory where you have cloned the repository, go to the 'src' folder, and select all of the files there. Then click 'Add'.
13. Click 'Debug', then 'Start Without Debugging'. The program will now be flashed onto the ATmega32 microcontroller.
14. Once the program has been transferred, you should see a message saying 'Initializing...' on the LCD. Disconnect the programmer and the 9V battery from the microcontroller.

Your microcontroller is now programmed and ready to be connected to the vehicle's OBD-II port. To connect the system to the car's OBD-II port, follow the steps below:

1. With the vehicle turned off, and the 9V battery disconnected, connect the wires labeled 'GND', 'K-Line', and '12V' in the schematic to pins 5, 7, and 16 of the vehicles's OBD-II port, respectively. [This diagram](https://github.com/arashn/obdii-reader/blob/master/diagrams/OBDII_connector.png) shows the numbering of the pins on the OBD-II port.
2. Once the microcontroller has been connected to the OBD-II port, start the vehicle. Then, connect the 9V battery to the microcontroller.
3. The LCD display should read 'Initializing...' for a few seconds. Then, the display will show the vehicle's current speed, in KM/h, and engine RPM. Pressing '1' on the keypad will change the display to show engine load and engine temperature. To go back to vehicle speed and engine RPM, press '1' again.

### Contributing
You can contribute to this project by helping add support for other protocols, and finding bugs. Please use the issue tracker to see current issues and file new bugs, or to request new features. You may fork this project and make your own additions or modifications to the system.
