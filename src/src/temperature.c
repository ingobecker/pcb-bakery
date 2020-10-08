#include <libopencm3/stm32/rtc.h>
#include "one_wire.h"
#include "temperature.h"

int one_wire_read_temp()
{
	// issue a temp conversion command
	int write_bytes[3] = { 0xcc, 0x44 };
	int read_bytes[9] = { 0 };
	if (one_wire_setup())
		one_wire_write_read(write_bytes, read_bytes, 2, 0);
	else
		return -1;

	// wait a second for temp conversion to complete
	uint32_t time = rtc_get_counter_val();
	while (time == rtc_get_counter_val())
		;

	// read the scratchpad
	write_bytes[1] = 0xbe;
	if (one_wire_setup())
		one_wire_write_read(write_bytes, read_bytes, 2, 9);
	else
		return -1;

	// check crc and convert byte from scatchpad to int, discarding
	// fractional digits.  only works for positive temperatures!
	if (one_wire_crc(read_bytes, 8) == read_bytes[8])
		return (read_bytes[1] << 4) | (read_bytes[0] >> 4);
	return 0;
}
