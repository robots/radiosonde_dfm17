#ifndef EEPROM_h_
#define EEPROM_h_

void eeprom_init(void);
int eeprom_read_buf(uint16_t addr, uint8_t *buf, size_t count);

#endif
