#pragma once
#include <stdint.h>

void spi_read(uint16_t addr, uint8_t len, uint8_t * buffer);
void spi_write(uint16_t addr, uint8_t len, uint8_t * buffer);
void spi_erase(uint16_t addr, uint8_t len);
