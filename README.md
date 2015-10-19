# obdii-reader
This is an OBDII diagnostic system for cars. It reads diagnostic information from a car's OBDII port. OBD stands for On-Board Diagnostic. All cars sold since 1996 in the United States are required to have an OBDII diagnostic system. These cars have an OBDII port, which has a 16-pin connector.

In the OBDII standard, there are five different protocols: ISO 9141-2, ISO 14230 KWP2000, SAE J1850 PWM, SAE J1850 VPW, and CAN. Some of the ways in which these protocols differ are the method of initializing communication, the number of bytes supported per packet, the baud rate used, and the OBDII connector pins used. J1850 PWM is used in Ford vehicles, whereas ISO 9141-2 and KWP2000 are mainly used in Chrysler vehicles, as well as European and Asian imports. For example, the 1996 Honda Accord uses the ISO 9141-2 protocol. Vehicles sold since 2008 are required to use the CAN protocol.

ISO 9141-2 and ISO 14230 KWP2000 very similar. ISO 9141-2 has a slow initialization method, while KWP2000 has both slow init and fast init methods. ISO 9141-2 and KWP2000's slow init methods are nearly identical, and they differ only in the value of the key bytes sent by the vehicle to the testing device. Both ISO 9141-2 and KWP2000 use pin 7 of the OBDII port, called the K-Line, for communication, and optionally, pin 15, called the L-Line, for sending a wake-up signal. Pin 16 provides 12V, and pins 4 and 5 are chassis ground and signal ground, respectively. Idle signal levels are high at 12V, and signals are active pull-down to 0V.

For the ISO 9141-2 slow init method, the sequence is as follows:

1. First, wait a minimum of 2600ms after the car is turned on, in order to prepare the ECU for a new initialization sequence.
2. Send byte 0x33 to the K-Line at 5 baud, using the standard serial communication routine (1 start bit, 8 character bits, no parity bit, and 1 stop bit).
3. Initialize UART to 10.4K baud, 8 data bits, no parity, and 1 stop.
4. Receive byte 0x55 from the vehicle.
5. Receive two key bytes, which are either 08 08 or 94 94 for ISO 9141-2. KWP2000's slow init method differs from ISO 9141-2 only in the value for these two key bytes.
6. Wait for about 40 milliseonds. Then, Invert key byte 2 and send to the vehicle. For example, key byte 0x08 becomes 0xF7.
7. Wait another 40 milliseconds. Then receive inverted address byte 0x33, which will be 0xCC.

At this point, initialization is complete. Before sending the first request, wait for about 65 milliseconds. Then, send the request, waiting about 10 milliseconds between each byte sent. Each byte that is sent is echoed back to the testing device as a confirmation of receipt of the byte. Therefore, after sending each byte, receive the echo byte before sending the next byte. You will also need to wait about 65ms after receiving the response for the last request before starting another request. Finally, to keep the connection open, you will need to make requests periodically, with a period of no more than 5000 ms between a request and the last response. Otherwise, the vehicle will close the connection with the testing device, and you will need to perform the initialization sequence again.

There are 9 modes, or services, available for use in the OBDII specification. However, many vehicles do not implement the higher services, so we will only use service 1 in this system, which returns enough useful information about the current state of the car. Within each service, there are several supported PIDs, or Parameter IDs. To see which PIDs are supported within a service, you can use one or more of the PIDs in that service. For example, to see which of the first 32 PIDs in Service 1 are supported, you can issue a Service 1 PID00 request. The result of the Service 1 PID00 request is 4 bytes, called A, B, C, and D, in that order. Collectively, these 4 bytes give a 32-bit number whose binary representation indicates which of the next 32 PIDs are supported by the car. For example, the 7th bit of byte A, going from MSB to LSB, indicates if Service 1 PID 07 is supported or not.

The response for other PIDs can have more or fewer bytes for the result, up to 12 bytes. However, Service 1 PIDs use at most 4 bytes for the result, which are A, B, C, and D.

The request data format for ISO 9141-2 is as follows:

```
[66 + cmdlen] [destination] [source]
[cmd(0)] [cmd(1)] ... [cmd(cmdlen - 1)]
[checksum]
```

The first 3 bytes are in the header part, the next `cmdlen` bytes are in the data part, and the last byte is the checksum part. `checksum` is the sum of the bytes in the header and data parts. `cmdlen` is the number of bytes needed for the request. All Service 1 requests use 2 bytes for the request, so `cmdlen` will be 2. For ISO 9141-2 the `destination` byte is 0x6A, and the `source` byte is 0xF1.

For a successful response, the response data format for ISO 9141-2 is as follows:

```
[42 + datalen] [destination] [source]
[40 + cmd(0)] [cmd(1)] ... [cmd(cmdlen - 1)] [result(0)] [result(1)] ...
[result(datalen - cmdlen - 1)]
[checksum]
```

`datalen` is the sum of the number of bytes for the request and the number of bytes for the response. For example, for a Service 1 PID00 request, there are 2 request bytes, 01 and 00, and there are 4 response bytes. In this case, `datalen` is 6.

For my system, since I only had access to a car that uses the ISO 9141-2 protocol, I decided to support that protocol only. Furthermore, I decided to display the engine RPM, vehicle speed, engine load, and engine coolant temperature, as well as the first 32 supported Service 1 PIDs. Pressing 1 on the keypad switches between showing the RPM and the speed, or the engine load and temperature. Pressing D on the keypad switches between showing the current vehicle information, like RPM and speed, or showing the result of the Service 1 PID00 request, which displays support for the first 32 Service 1 PIDs.

Remarks:
The most difficult part of this project for me was figuring out the exact protocol and procedure for communicating with a car, such as which bytes to send to the car, which bytes to expect from the car, the timing of bytes sent and received, and how to decode the meaning of the bytes sent and received. As the use of the OBDII system has been made easy by the myriad scan tools available, there is scant information about how to build a custom diagnostic reader. There was no well-defined tutorial on how to interface with a car's OBDII system using an AVR microcontroller. As such, I had to piece together information from the web, reading documents regarding OBDII specifications and protocols, and incorporating my own knowledge about networking protocols and definitions, in order to come up with the correct setup for my particular system.

### Credits
Some of the sources that helped me build this project:

* ODB II » Tech Toy Hacks: http://blog.perquin.com/blog/category/odbii/
* K-line Communication Description - OBD Clearinghouse: http://www.obdclearinghouse.com/index.php?body=get_file&id=1343
* OnBoardDiagnostics.com - OBD-II Network Standards: http://www.onboarddiagnostics.com/page03.htm
* On-board diagnostics - Wikipedia, the free encyclopedia: http://en.wikipedia.org/wiki/On-board_diagnostics
* Creating A Wireless OBDII Scanner: https://www.wpi.edu/Pubs/E-project/Available/E-project-101808-133424/unrestricted/FinalPaper.pdf
* Initializing communication to vehicle OBDII System: http://fett.tu-sofia.bg/et/2005/pdf/Paper097-P_Dzhelekarski1.pdf
* Freediag - Vehicle Diagnostic Suite: http://freediag.sourceforge.net/
