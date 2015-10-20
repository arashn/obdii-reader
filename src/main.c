/**
 *  OBD-II Reader - An OBD-II diagnostics system based on the ATmega32
 *  Copyright (C) 2015  Arash Nabili
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * For installation and setup instructions, please see the README.md file.
 */

#include <asf.h>
#include <stdio.h>
#include "avr.h"
#include "avr.c"
#include "lcd.h"
#include "lcd.c"

char buf0[17];
char buf1[17];

unsigned char s1pid00[10]; // Supported PIDs

unsigned char load; // Engine load
signed char temperature; // Engine coolant temperature
unsigned short rpm; // Instantaneous engine RPM
unsigned char speed; // Instantaneous vehicle speed
unsigned char mode;
unsigned char page;
volatile unsigned char timer_flag;

void timer_setup(void); // Setup for timer interrupt
void update_lcd(void);
unsigned char get_key(void);
unsigned char key_pressed(unsigned char r, unsigned char c);
void USART_Init(unsigned int baud); // Initialize the USART
void USART_Transmit(unsigned char data); // Transmit using the USART
unsigned char USART_Receive(void); // Receive using the USART
void obd_init(void); // OBDII ISO 9141-2 initialization sequence

void timer_setup(void) {
	cli();
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1B |= (1 << WGM12);
	TIMSK |= (1 << OCIE1A);
	sei();
	OCR1A = 15624; // Interrupt once every 500 ms (2Hz)
	TCCR1B |= (1 << CS12);
}

ISR(TIMER1_COMPA_vect) {
	timer_flag = 1;
}

void update_lcd(void) {
	if (mode == 1) { // Show first 32 supported Service 1 PIDs 
		sprintf(buf0, "%02X %02X %02X %02X %02X", (unsigned int)(s1pid00[0] & 0xFF), (unsigned int)(s1pid00[1] & 0xFF), (unsigned int)(s1pid00[2] & 0xFF),
				(unsigned int)(s1pid00[3] & 0xFF), (unsigned int)(s1pid00[4] & 0xFF));
		sprintf(buf1, "%02X %02X %02X %02X %02X", (unsigned int)(s1pid00[5] & 0xFF), (unsigned int)(s1pid00[6] & 0xFF), (unsigned int)(s1pid00[7] & 0xFF),
				(unsigned int)(s1pid00[8] & 0xFF), (unsigned int)(s1pid00[9] & 0xFF));
	}
	else if (page == 0) { // Show first page: RPM and speed
		sprintf(buf0, "RPM: %i", rpm);
		sprintf(buf1, "KM/H: %i", speed);
	}
	else { // Show second page: Engine load and engine coolant temperature
		sprintf(buf0, "Load: %i", load);
		sprintf(buf1, "Temp: %i", temperature);
	}
	
	clr_lcd();
	pos_lcd(0, 0);
	puts_lcd2(buf0);
	pos_lcd(1, 0);
	puts_lcd2(buf1);
}

unsigned char get_key(void) {
	unsigned char r, c;
	for (r = 0; r < 4; ++r) {
		for (c = 0; c < 4; ++c) {
			if (key_pressed(r, c)) {
				return (r * 4) + c + 1;
			}
		}
	}
	return 0;
}

unsigned char key_pressed(unsigned char r, unsigned char c) {
	DDRC = 0;
	PORTC = 0;
	CLR_BIT(DDRC, r);
	SET_BIT(PORTC, r);
	SET_BIT(DDRC, c + 4);
	CLR_BIT(PORTC, c + 4);
	if (!GET_BIT(PINC, r)) {
		return 1;
	}
	else {
		return 0;
	}
}

void USART_Init(unsigned int baud) {
	// Set baud rate
	UBRRH = (unsigned char) (baud >> 8);
	UBRRL = (unsigned char) baud;
	
	// Enable receiver and transmitter
	UCSRB = (1 << RXEN) | (1 << TXEN);
	
	// Set frame format: 8data, 1stop bit, no parity
	UCSRC = (1 << URSEL) | (3 << UCSZ0);
}

void USART_Transmit(unsigned char data) {
	// Wait for empty transmit buffer
	while (!(UCSRA & (1 << UDRE)));
	
	// Put data into buffer, sends the data
	UDR = data;
}

unsigned char USART_Receive(void) {
	// Wait for data to be received
	while (!(UCSRA & (1 << RXC)));
	
	// Get and return received data from buffer
	return UDR;
}

void obd_init(void) {
	// Using ISO 9141-2 Protocol with 5-baud initialization
	
	sprintf(buf0, "Initializing...");
	pos_lcd(0, 0);
	puts_lcd2(buf0);
	
	DDRD |= (1 << 1); // Set Pin PD1 as output
	
	PORTD |= (1 << 1); // Write 1 to PD1
	
	wait_avr(2610); // Wait 2610 ms for ECU to reset fully
	
	// Send a byte 33 hex at 5 baud
	PORTD &= ~(1 << 1);
	wait_avr(200);
	PORTD |= (1 << 1);
	wait_avr(400);
	PORTD &= ~(1 << 1);
	wait_avr(400);
	PORTD |= (1 << 1);
	wait_avr(400);
	PORTD &= ~(1 << 1);
	wait_avr(400);
	PORTD |= (1 << 1);
	wait_avr(200);
	
	// Initialize serial connection
	// ~10400 baud --> UBRR = 47
	// 8-bit character size
	// No parity bit, 1 stop bit
	USART_Init(47);
	
	// Wait for response: Byte 55 hex
	unsigned char response = USART_Receive();
	
	// Receive two key bytes
	// 08 08, 94 94 for ISO 9141
	// 8F E9, 8F 6B, 8F 6D, 8F EF for KWP
	unsigned char keyByte1 = USART_Receive();
	unsigned char keyByte2 = USART_Receive();
	
	wait_avr(40);
	
	// Send ACK: Inverted key byte 2
	unsigned char ack1 = ~keyByte2;
	USART_Transmit(ack1);
	
	wait_avr(40);
	
	// Receive ACK from vehicle: Inverted address 33
	unsigned char ack2 = USART_Receive();
	ack2 = USART_Receive();
	
	clr_lcd();
}

int main (void)
{
	board_init();
	ini_lcd();
	obd_init();
	timer_setup();
	
	cli(); // Disable interrupts
	
	wait_avr(100);
	
	unsigned char keyPressed;
	mode = 0; // Show vehicle information / supported PIDs
	page = 0; // Show RPM and speed / engine load and engine coolant temperature
	
	unsigned char s1pid04[7]; // Engine load
	unsigned char s1pid05[7]; // Engine coolant temperature
	unsigned char s1pid0c[8]; // Engine RPM
	unsigned char s1pid0d[7]; // Vehicle speed
	
	// OBDII Initialized; Send Service 1 PID 00 request message
	// 68 6A F1 01 00 C4
	unsigned char receive;
	
	USART_Transmit(0x68);
	receive = USART_Receive();
	wait_avr(10);
	USART_Transmit(0x6A);
	receive = USART_Receive();
	wait_avr(10);
	USART_Transmit(0xF1);
	receive = USART_Receive();
	wait_avr(10);
	USART_Transmit(0x01);
	receive = USART_Receive();
	wait_avr(10);
	USART_Transmit(0x00);
	receive = USART_Receive();
	wait_avr(10);
	USART_Transmit(0xC4);
	receive = USART_Receive();
	
	// Service 1 PID 00 response
	int i = 0;
	for (i = 0; i < 10; ++i) {
		s1pid00[i] = USART_Receive();
	}
	
	sei(); // Enable interrupts
	
	while (1) {
		wait_avr(65);
		
		// Request Service 1 PID 04: Engine load
		// 68 6A F1 01 04 C8
		USART_Transmit(0x68);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0x6A);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0xF1);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0x01);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0x04);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0xC8);
		receive = USART_Receive();
		
		// Service 1 PID 04 response
		for (i = 0; i < 7; ++i) {
			s1pid04[i] = USART_Receive();
		}
		
		wait_avr(65);
		
		// Request Service 1 PID 05: Engine coolant temperature
		// 68 6A F1 01 05 C9
		USART_Transmit(0x68);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0x6A);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0xF1);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0x01);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0x05);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0xC9);
		receive = USART_Receive();
		
		// Service 1 PID 05 response
		for (i = 0; i < 7; ++i) {
			s1pid05[i] = USART_Receive();
		}
		
		wait_avr(65);
		
		// Request Service 1 PID 0C: Engine RPM
		// 68 6A F1 01 0C D0
		USART_Transmit(0x68);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0x6A);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0xF1);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0x01);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0x0C);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0xD0);
		receive = USART_Receive();
		
		// Service 1 PID 0C response
		for (i = 0; i < 8; ++i) {
			s1pid0c[i] = USART_Receive();
		}
		
		wait_avr(65);
		
		// Request Service 1 PID 0D: Vehicle speed
		// 68 6A F1 01 0D D1
		USART_Transmit(0x68);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0x6A);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0xF1);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0x01);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0x0D);
		receive = USART_Receive();
		wait_avr(10);
		USART_Transmit(0xD1);
		receive = USART_Receive();
		
		// Service 1 PID 0D response
		for (i = 0; i < 7; ++i) {
			s1pid0d[i] = USART_Receive();
		}
		
		keyPressed = get_key();
		if (keyPressed == 1) {
			page ^= 1;
		}
		else if (keyPressed == 16) {
			mode ^= 1;
		}
		
		load = s1pid04[5] * 100 / 255; // Engine load is A * 100 / 255, in percent
		temperature = s1pid05[5] - 40; // Engine coolant temperature is A - 40, in Celsius
		rpm = (s1pid0c[5] * 256 + s1pid0c[6]) / 4; // RPM is ((A * 256) + B) / 4
		speed = s1pid0d[5]; // Vehicle speed is A, in Km/h
		while (!timer_flag);
		timer_flag = 0;
		update_lcd();
	}
}
