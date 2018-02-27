// Board-Config
#include "OnBoard.h"

// Application
#include "BME280.h"

// Drivers
#include "hal_i2c.h"

// Temperature compensation
uint16 dig_T1 = 0;
int16 dig_T2 = 0;
int16 dig_T3 = 0;

// Pressure compensation
uint16 dig_P1 = 0;
int16 dig_P2 = 0;
int16 dig_P3 = 0;
int16 dig_P4 = 0;
int16 dig_P5 = 0;
int16 dig_P6 = 0;
int16 dig_P7 = 0;
int16 dig_P8 = 0;
int16 dig_P9 = 0;

// Humidity compensation
uint8 dig_H1 = 0;
int16 dig_H2 = 0;
uint8 dig_H3 = 0;
int16 dig_H4 = 0;
int16 dig_H5 = 0;
int8 dig_H6 = 0;


uint8 init_BME280 (void) {
  HalI2CInit (i2cClock_267KHZ);
  
  uint8 ACK = 0;
  uint8 writeBuffer [2];
  uint8 readBuffer  [25];
  
  //writeBuffer[0] = 0xE0;        // Reset-Register
  //writeBuffer[1] = 0xB6;        // Reset
  //HalI2CWrite (Si7021_I2C_ADDRESS, 2, writeBuffer);
  
  writeBuffer[0] = 0xF2;        // Hum-Oversampling (2<-0)
  writeBuffer[1] = 0x01;        // Value
  ACK = HalI2CWrite (BME280_I2C_ADDRESS, 2, writeBuffer);
  if (ACK == 0) return 0;
  
  writeBuffer[0] = 0xF4;        // Temp-Oversampling (7<-5), Press-Oversampling (4<-2), Samping-Mode (1<-0)
  writeBuffer[1] = 0x24;        // Value
  ACK = HalI2CWrite (BME280_I2C_ADDRESS, 2, writeBuffer);
  if (ACK == 0) return 0;
  
  writeBuffer[0] = 0xF5;        // Filter-Time (7<-5), Filter-Coeff (4<-2), SPI-Enable (0)
  writeBuffer[1] = 0x00;        // Value
  ACK = HalI2CWrite (BME280_I2C_ADDRESS, 2, writeBuffer);
  if (ACK == 0) return 0;
  
  writeBuffer[0] = 0x88;        // dig_T1 - Register
  ACK = HalI2CWrite (BME280_I2C_ADDRESS, 1, writeBuffer);
  if (ACK == 0) return 0;
  HalI2CRead  (BME280_I2C_ADDRESS, 24, readBuffer);
  
  // Temperature compensation
  dig_T1 = readBuffer [0] | (readBuffer [1] << 8);
  dig_T2 = readBuffer [2] | (readBuffer [3] << 8);
  dig_T3 = readBuffer [4] | (readBuffer [5] << 8);

  // Pressure compensation
  dig_P1 = readBuffer [6] | (readBuffer [7] << 8);
  dig_P2 = readBuffer [8] | (readBuffer [9] << 8);
  dig_P3 = readBuffer [10] | (readBuffer [11] << 8);
  dig_P4 = readBuffer [12] | (readBuffer [13] << 8);
  dig_P5 = readBuffer [14] | (readBuffer [15] << 8);
  dig_P6 = readBuffer [16] | (readBuffer [17] << 8);
  dig_P7 = readBuffer [18] | (readBuffer [19] << 8);
  dig_P8 = readBuffer [20] | (readBuffer [21] << 8);
  dig_P9 = readBuffer [22] | (readBuffer [23] << 8);

  writeBuffer[0] = 0xA1;        // dig_H1 - Register
  ACK = HalI2CWrite (BME280_I2C_ADDRESS, 1, writeBuffer);
  if (ACK == 0) return 0;
  HalI2CRead  (BME280_I2C_ADDRESS, 1, readBuffer);
  
  // Humidity compensation
  dig_H1 = readBuffer [0];
  
  writeBuffer[0] = 0xE1;        // dig_H2 - Register
  ACK = HalI2CWrite (BME280_I2C_ADDRESS, 1, writeBuffer);
  if (ACK == 0) return 0;
  HalI2CRead  (BME280_I2C_ADDRESS, 7, readBuffer);
  
  // Humidity compensation
  dig_H2 = readBuffer [0] | (readBuffer [1] << 8);
  dig_H3 = readBuffer [2];
  dig_H4 = (readBuffer [3] << 4) | (readBuffer [4] & 0xF0);
  dig_H5 = (readBuffer [4] >> 4) | (readBuffer [5] << 4);
  dig_H6 = readBuffer [6];
  
  HalI2CEnterSleep ();
  return 1;
}

uint8 start_BME280 (void) {
  HalI2CExitSleep ();
  
  uint8 ACK = 0;
  uint8 writeBuffer [2];
  
  // Force single meassure
  writeBuffer[0] = 0xF4;        // Temp-Oversampling (7<-5), Press-Oversampling (4<-2), Samping-Mode (1<-0)
  writeBuffer[1] = 0x25;        // Value
  ACK = HalI2CWrite ( BME280_I2C_ADDRESS, 2, writeBuffer );
  
  HalI2CEnterSleep ();
  
  if (ACK == 0) return 0;
  else return 1;
}

uint8 read_BME280 (int16 *temp, uint16 *hum, uint16 *press) {
  HalI2CExitSleep ();
  
  int32  tTemp  = 0;
  int32  tHum   = 0;
  uint32 tPress = 0;
  
  uint8 ACK = 0;
  uint8 readBuffer [10];
  uint8 writeBuffer [1];
  
  // Read out values
  writeBuffer[0] = 0xF7;
  ACK = HalI2CWrite (BME280_I2C_ADDRESS, 1, writeBuffer);
  if (ACK == 0) return 0;
  
  HalI2CRead  (BME280_I2C_ADDRESS, 9, readBuffer );
  tTemp  = ((uint32)readBuffer[3] << 12) | ((uint32)readBuffer[4] << 4) | ((uint32)readBuffer[5] >> 4);
  tHum   = ((uint32)readBuffer[6] << 8) | ((uint32)readBuffer[7] << 4);
  tPress = ((uint32)readBuffer[0] << 12) | ((uint32)readBuffer[1] << 4) | ((uint32)readBuffer[2] >> 4);
  
  // Convert readout to units
  tTemp = BME280_compensate_T_int32 ( tTemp );
  tHum = (int32)BME280_compensate_H_double ( tHum );
  tPress = BME280_compensate_P_int32 ( tPress );
  
  // Convert to uint16
  *temp  = (int16)(tTemp);
  *hum   = (uint16)(tHum);
  *press = (uint16)(tPress / 10); 
  
  HalI2CEnterSleep ();
  return 1;
}

// Returns temperature in DegC, double precision. Output value of “51.23” equals 51.23 DegC.
// t_fine carries fine temperature as global value
BME280_S32_t t_fine;
double BME280_compensate_T_double (int32 adc_T) {
  double var1, var2, T;
  var1 = (((double) adc_T) / 16384.0 - ((double)dig_T1) / 1024.0) * ((double)dig_T2);
  var2 = ((((double)adc_T)/131072.0 - ((double)dig_T1)/8192.0) * (((double)adc_T)/131072.0 - ((double) dig_T1)/8192.0)) * ((double)dig_T3);
  t_fine = (int32)(var1 + var2);
  T = (var1 + var2) / 5120.0;
  return T;
}

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
BME280_S32_t BME280_compensate_T_int32 (BME280_S32_t adc_T) {
  int32 var1, var2, T;
  var1 = ((((adc_T>>3) - ((BME280_S32_t)dig_T1<<1))) * ((BME280_S32_t)dig_T2)) >> 11;
  var2 = (((((adc_T>>4) - ((BME280_S32_t)dig_T1)) * ((adc_T>>4) - ((BME280_S32_t)dig_T1))) >> 12) * ((BME280_S32_t)dig_T3)) >> 14;
  t_fine = var1 + var2;
  T = (t_fine * 5 + 128) >> 8;
  return T;
}

// Returns humidity in %rH as as double. Output value of “4633.2” represents 46.332 %rH
double BME280_compensate_H_double (int32 adc_H) {
  double var_H;
  var_H = (((double)t_fine) - 76800.0);
  var_H = (adc_H - (((double)dig_H4) * 64.0 + ((double)dig_H5) / 16384.0 * var_H)) * (((double)dig_H2) / 65536.0 * (1.0 + ((double)dig_H6) / 67108864.0 * var_H * (1.0 + ((double)dig_H3) / 67108864.0 * var_H)));
  var_H = var_H * (1.0 - ((double)dig_H1) * var_H / 524288.0);
  if (var_H > 100.0)
    var_H = 100.0;
  else if (var_H < 0.0)
    var_H = 0.0;
  return var_H * 100;
}

// Returns pressure in Pa as double. Output value of “96386.2” equals 96386.2 Pa = 963.862 hPa
double BME280_compensate_P_double (int32 adc_P) {
  double var1, var2, p;
  var1 = ((double)t_fine / 2.0) - 64000.0;
  var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
  var2 = var2 + var1 * ((double)dig_P5) * 2.0;
  var2 = (var2/4.0)+(((double)dig_P4) * 65536.0);
  var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + ((double)dig_P2) * var1) / 524288.0;
  var1 = (1.0 + var1 / 32768.0)*((double)dig_P1);
  if (var1 == 0.0) {
    return 0; // avoid exception caused by division by zero
  }
  p = 1048576.0 - (double)adc_P;
  p = (p - (var2 / 4096.0)) * 6250.0 / var1;
  var1 = ((double)dig_P9) * p * p / 2147483648.0;
  var2 = p * ((double)dig_P8) / 32768.0;
  p = p + (var1 + var2 + ((double)dig_P7)) / 16.0;
  return p;
}

// Returns pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
BME280_U32_t BME280_compensate_P_int32 (BME280_S32_t adc_P) {
  BME280_S32_t var1, var2;
  BME280_U32_t p;
  var1 = (((BME280_S32_t)t_fine)>>1) - (BME280_S32_t)64000;
  var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((BME280_S32_t)dig_P6);
  var2 = var2 + ((var1*((BME280_S32_t)dig_P5))<<1);
  var2 = (var2>>2)+(((BME280_S32_t)dig_P4)<<16);
  var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((BME280_S32_t)dig_P2) * var1)>>1))>>18;
  var1 =((((32768+var1))*((BME280_S32_t)dig_P1))>>15);
  if (var1 == 0) {
    return 0; // avoid exception caused by division by zero
  }
    p = (((BME280_U32_t)(((BME280_S32_t)1048576)-adc_P)-(var2>>12)))*3125;
  if (p < 0x80000000) {
    p = (p << 1) / ((BME280_U32_t)var1);
  }
  else {
    p = (p / (BME280_U32_t)var1) * 2;
  }
  var1 = (((BME280_S32_t)dig_P9) * ((BME280_S32_t)(((p>>3) * (p>>3))>>13)))>>12;
  var2 = (((BME280_S32_t)(p>>2)) * ((BME280_S32_t)dig_P8))>>13;
  p = (BME280_U32_t)((BME280_S32_t)p + ((var1 + var2 + dig_P7) >> 4));
  return p;
}