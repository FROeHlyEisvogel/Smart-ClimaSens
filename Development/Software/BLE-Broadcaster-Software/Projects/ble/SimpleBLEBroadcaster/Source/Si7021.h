#ifndef Si7021_H
#define Si7021_H

#define Si7021_I2C_ADDRESS      0x40
#define Si7021_DEVICE_INDICATOR 0x14

#define HAL_I2C                 TRUE
#define HAL_I2C_MASTER          TRUE

uint8 init_Si7021 (void);
uint8 read_SerialNumber_Si7021 (void);
uint8 start_Si7021 (void);
uint8 read_Si7021 (int16 *temp, uint16 *hum);

#endif /* Si7021_H */