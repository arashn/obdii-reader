#include "avr.h"
#include "lcd.h"

#define DDR     DDRB
#define PORT    PORTB
#define RS_PIN  0
#define RW_PIN  1
#define EN_PIN  2

static inline void
set_data(unsigned char x)
{
	DDRD |= 0xf0;
	PORTD &= 0x0f;
	PORTD |= x;
}

static inline unsigned char
get_data(void)
{
	DDRD &= 0x0f;
	return PIND & 0xf0;
}

static inline void
sleep_700ns(void)
{
	NOP();
	NOP();
	NOP();
}

static unsigned char
input(unsigned char rs)
{
	unsigned char d, temp;
	if (rs) SET_BIT(PORT, RS_PIN); else CLR_BIT(PORT, RS_PIN);
	SET_BIT(PORT, RW_PIN);
	get_data();
	SET_BIT(PORT, EN_PIN);
	sleep_700ns();
	d = get_data();
	CLR_BIT(PORT, EN_PIN);
	SET_BIT(PORT, EN_PIN);
	sleep_700ns();
	temp = get_data();
	temp = temp >> 4;
	temp &= 0x0f;
	d |= temp;
	CLR_BIT(PORT, EN_PIN);
	return d;
}

static void
output(unsigned char d, unsigned char rs)
{
	if (rs) SET_BIT(PORT, RS_PIN); else CLR_BIT(PORT, RS_PIN);
	CLR_BIT(PORT, RW_PIN);
	set_data(d);
	SET_BIT(PORT, EN_PIN);
	sleep_700ns();
	CLR_BIT(PORT, EN_PIN);
}

static void
write(unsigned char c, unsigned char rs)
{
	unsigned char temp = c;
	while (input(0) & 0x80);
	temp &= 0xf0;
	output(temp, rs);
	temp = c << 4;
	temp &= 0xf0;
	output(temp, rs);
}

void
ini_lcd(void)
{
	SET_BIT(DDR, RS_PIN);
	SET_BIT(DDR, RW_PIN);
	SET_BIT(DDR, EN_PIN);
	wait_avr(16);
	output(0x30, 0);
	wait_avr(5);
	output(0x30, 0);
	wait_avr(1);
	output(0x30, 0);
	while (input(0) & 0x80);
	output(0x20, 0);
	write(0x2c, 0);
	write(0x08, 0);
	write(0x01, 0);
	write(0x06, 0);
	write(0x0c, 0);
}

void
clr_lcd(void)
{
	write(0x01, 0);
}

void
pos_lcd(unsigned char r, unsigned char c)
{
	unsigned char n = r * 40 + c;
	write(0x02, 0);
	while (n--) {
		write(0x14, 0);
	}
}

void
put_lcd(char c)
{
	write(c, 1);
}

void
puts_lcd1(const char *s)
{
	char c;
	while ((c = pgm_read_byte(s++)) != 0) {
		write(c, 1);
	}
}

void
puts_lcd2(const char *s)
{
	char c;
	while ((c = *(s++)) != 0) {
		write(c, 1);
	}
}