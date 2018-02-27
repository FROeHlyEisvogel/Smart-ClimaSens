#ifndef HTU21D_H
#define HTU21D_H

#define HTU21D_I2C_ADDRESS      0x40

#define HAL_I2C                 TRUE
#define HAL_I2C_MASTER          TRUE

uint8 init_HTU21D (void);
uint8 measure_humidity_HTU21D (void);
uint8 measure_temperature_HTU21D (void);
uint8 read_temperature_HTU21D (int16 *temp);
uint8 read_humidity_HTU21D (uint16 *hum);
uint8 soft_reset_HTU21D (void);

#endif /* HTU21D_H */