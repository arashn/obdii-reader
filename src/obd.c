#include "obd.h"
#include "avr.h"
#include <stdlib.h>

// Parameters specific to ISO 9141-2
const unsigned char CMD_LEN_OFFSET = 0x66;
const unsigned char DESTINATION = 0x6A;
const unsigned char SOURCE = 0xF1;
const unsigned char DATA_LEN_OFFSET = 0x42;

static inline
int send_byte(unsigned char data)
{
	// Send byte and receive response byte (same as sent byte) as ACK
	// Return error if response != sent byte
	unsigned char receive;
	USART_Transmit(data);
	receive = USART_Receive();
	
	return receive == data ? 0 : -1;
}

static inline
int send_cmd(const unsigned char cmd[], size_t cmd_len, unsigned char result_buf[], size_t result_len)
{
	unsigned char receive;
	int i;
	unsigned char checksum;
	unsigned char data_len;

	checksum = 0x0;

	if (!send_byte(CMD_LEN_OFFSET + cmd_len)) {
		return -1;
	}
	checksum = checksum + CMD_LEN_OFFSET + cmd_len;

	wait_avr(10);
	if (!send_byte(DESTINATION)) {
		return -1;
	}
	checksum = checksum + DESTINATION;

	wait_avr(10);
	if (!send_byte(SOURCE)) {
		return -1;
	}
	checksum = checksum + SOURCE;

	for (i = 0; i < cmd_len; ++i) {
		wait_avr(10);
		if (!send_byte(cmd[i])) {
			return -1;
		}
		checksum = checksum + cmd[i];
	}

	wait_avr(10);
	if (!send_byte(checksum)) {
		return -1;
	}

	// Receive response
	receive = USART_Receive();
	data_len = receive - DATA_LEN_OFFSET;
	if (data_len - cmd_len != result_len) {
		return -1;
	}

	receive = USART_Receive();
	if (receive != DESTINATION) {
		return -1;
	}

	receive = USART_Receive();
	if (receive != SOURCE) {
		return -1;
	}

	for (i = 0; i < cmd_len; ++i) {
		receive = USART_Receive();
		if (receive != cmd[i]) {
			return -1;
		}
	}

	for (i = 0; i < result_len; ++i) {
		result_buf[i] = USART_Receive();
	}

	receive = USART_Receive(); // TODO: This is checksum in response. Check if it's correct. (Checksum of what?)

	return 0;
}

static inline
int get_engine_load(obd_info_t *obd_info)
{
	// Request Service 1 PID 04: Engine load (01 04)
	// 68 6A F1 01 04 C8

	unsigned char cmd[2] = {0x01, 0x04};
	unsigned char result[1];
	if (!send_cmd(cmd, 2, result, 1)) {
		obd_info->load = 0;
		return -1;
	}

	obd_info->load = result[0] * 100 / 255; // Engine load is A * 100 / 255, in percent

	return 0;
}

static inline
int get_engine_coolant_temp(obd_info_t *obd_info)
{
	// Request Service 1 PID 05: Engine coolant temperature (01 05)
	// 68 6A F1 01 05 C9

	unsigned char cmd[2] = {0x01, 0x05};
	unsigned char result[1];
	if (!send_cmd(cmd, 2, result, 1)) {
		obd_info->temperature = 0;
		return -1;
	}

	obd_info->temperature = result[0] - 40; // Engine coolant temperature is A - 40, in Celsius

	return 0;
}

static inline
int get_engine_rpm(obd_info_t *obd_info)
{
	// Request Service 1 PID 0C: Engine RPM (01 0C)
	// 68 6A F1 01 0C D0

	unsigned char cmd[2] = {0x01, 0x0C};
	unsigned char result[2];
	if (!send_cmd(cmd, 2, result, 2)) {
		obd_info->rpm = 0;
		return -1;
	}

	obd_info->rpm = (result[0] * 256 + result[1]) / 4; // RPM is ((A * 256) + B) / 4

	return 0;
}

static inline
int get_vehicle_speed(obd_info_t *obd_info)
{
	// Request Service 1 PID 0D: Vehicle speed (01 0D)
	// 68 6A F1 01 0D D1

	unsigned char cmd[2] = {0x01, 0x0D};
	unsigned char result[1];
	if (!send_cmd(cmd, 2, result, 1)) {
		obd_info->speed = 0;
		return -1;
	}

	obd_info->speed = result[0]; // Vehicle speed is A, in Km/h

	return 0;
}

void obd_init(void)
{
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

int get_service1_supported_pids(obd_info_t *obd_info)
{
	// Send Service 1 PID 00 request message (01 00)
	// 68 6A F1 01 00 C4

	unsigned char cmd[2] = {0x01, 0x00};
	if (!send_cmd(cmd, 2, obd_info->s1pid00, 4)) {
		return -1;
	}

	return 0;
}

int get_obd_data(obd_info_t *obd_info) {
	wait_avr(65);
	get_engine_load(obd_info);
	wait_avr(65);
	get_engine_coolant_temp(obd_info);
	wait_avr(65);
	get_engine_rpm(obd_info);
	wait_avr(65);
	get_vehicle_speed(obd_info);

	return 0;
}