/**
 *  OBD-II Reader - An OBD-II diagnostics system based on the ATmega32
 *  Copyright (C) 2022  Arash Nabili
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
#include "lcd.h"
#include "usart.h"
#include "obd.h"

char buf0[17];
char buf1[17];

unsigned char mode;
unsigned char page;
volatile unsigned char timer_flag;

obd_info_t obd_info;

void timer_setup(void); // Setup for timer interrupt
void update_lcd(void);
unsigned char get_key(void);
unsigned char key_pressed(unsigned char r, unsigned char c);

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
		sprintf(buf0, "%02X %02X %02X %02X",
				(unsigned int)(obd_info.s1pid00[0] & 0xFF), (unsigned int)(obd_info.s1pid00[1] & 0xFF),
				(unsigned int)(obd_info.s1pid00[2] & 0xFF), (unsigned int)(obd_info.s1pid00[3] & 0xFF));
	}
	else if (page == 0) { // Show first page: RPM and speed
		sprintf(buf0, "RPM: %i", obd_info.rpm);
		sprintf(buf1, "KM/H: %i", obd_info.speed);
	}
	else { // Show second page: Engine load and engine coolant temperature
		sprintf(buf0, "Load: %i", obd_info.load);
		sprintf(buf1, "Temp: %i", obd_info.temperature);
	}
	
	clr_lcd();
	pos_lcd(0, 0);
	puts_lcd2(buf0);
	if (mode == 0) {
		pos_lcd(1, 0);
		puts_lcd2(buf1);
	}
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

int main (void)
{
	board_init();
	ini_lcd();
	
	sprintf(buf0, "Initializing...");
	pos_lcd(0, 0);
	puts_lcd2(buf0);

	obd_init(); // Using ISO 9141-2 Protocol with 5-baud initialization
	timer_setup();
	
	cli(); // Disable interrupts
	
	wait_avr(100);
	
	unsigned char keyPressed;
	mode = 0; // Show vehicle information / supported PIDs
	page = 0; // Show RPM and speed / engine load and engine coolant temperature
	
	// OBDII Initialized
	get_service1_supported_pids(&obd_info);
	
	sei(); // Enable interrupts
	
	while (1) {
		get_obd_data(&obd_info);
		
		keyPressed = get_key();
		if (keyPressed == 1) {
			page ^= 1;
		}
		else if (keyPressed == 16) {
			mode ^= 1;
		}

		while (!timer_flag);
		timer_flag = 0;
		update_lcd();
	}
}
