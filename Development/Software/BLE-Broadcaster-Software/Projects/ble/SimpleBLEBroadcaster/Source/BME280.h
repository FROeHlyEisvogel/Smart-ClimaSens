#ifndef BME280_H
#define BME280_H

#define BME280_S32_t int32
#define BME280_U32_t uint32

#define BME280_I2C_ADDRESS      0x76 // or 0x77

#define HAL_I2C                 TRUE
#define HAL_I2C_MASTER          TRUE

uint8 init_BME280 (void);
uint8 start_BME280 (void);
uint8 read_BME280 (int16 *temp, uint16 *hum, uint16 *press);

double BME280_compensate_T_double (BME280_S32_t adc_T);
double BME280_compensate_P_double (BME280_S32_t adc_P);
double BME280_compensate_H_double (BME280_S32_t adc_H);

BME280_S32_t BME280_compensate_T_int32 (BME280_S32_t adc_T);
BME280_U32_t BME280_compensate_P_int32 (BME280_S32_t adc_P);

#endif /* BME280_H */