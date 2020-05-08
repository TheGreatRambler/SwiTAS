#pragma once
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/gpio.h>

#define NUNCHUK_DEVICE_ID 0x52

#define I2C_TIMEOUT 1000
#define ERR_TIMEOUT -1

enum
{
    CTRLR_UNINITILIZED,
    CTRLR_PASS_1,
    CTRLR_INITILIZED,
    CTRLR_PRESENT,
    CTRLR_TIMEOUT,
    CTRLR_UNPLUGGED
};

typedef struct
{
    uint8_t id;
    uint32_t i2c;
    uint16_t clk;
    uint16_t gpios;
    uint8_t state;
    uint8_t packet[8];
} snes_i2c_state;

extern snes_i2c_state controller_1;
extern snes_i2c_state controller_2;

void sns_init(snes_i2c_state *controller);
void sns_request(snes_i2c_state *controller);
void sns_poll(snes_i2c_state *controller);

// pinout
//power nc   data
//clk   nc gpd
//clk power  nc  data  gnd

// ok
// pb6 3v scl gnd