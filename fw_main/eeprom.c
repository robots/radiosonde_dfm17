
#include "platform.h"

#include "i2c.h"

#include "eeprom.h"

#define EEPROM_ADDR 0x53

void eeprom_init(void)
{
}

int eeprom_read_buf(uint16_t addr, uint8_t *buf, size_t count)
{
	int ret;
	uint8_t addr_ary[2] = {addr >> 8u, addr & 0xffu};

	// write i2c addr
	ret = i2c_start();
	if (ret != 0)
	{
		i2c_stop();
		return 1;
	}

	ret = i2c_write_addr(EEPROM_ADDR);
	// ack required
	if (ret != 0)
	{
		i2c_stop();
		return 1;
	}

	// write memory address 16bit;
	ret = i2c_write(addr_ary, 2);
	// ack required
	if (ret != 0)
	{
		i2c_stop();
		return 1;
	}
	i2c_stop();

	// send new start sequence with read address, read is autostop
	ret = i2c_start();
	if (ret != 0)
	{
		i2c_stop();
		return 1;
	}

	ret = i2c_read(EEPROM_ADDR, buf, count);
	// ack required
	if (ret != 0)
	{
		i2c_stop();
		return 1; 
	}

	return 0;
}

