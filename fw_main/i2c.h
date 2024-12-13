#ifndef I2C_h_
#define I2C_h_

void i2c_init(void);
int i2c_start(void);
int i2c_stop(void);
int i2c_write_addr(uint8_t addr);
int i2c_write(uint8_t *data, uint8_t count);
int i2c_read(uint8_t addr, uint8_t *data, uint8_t count);

#endif
