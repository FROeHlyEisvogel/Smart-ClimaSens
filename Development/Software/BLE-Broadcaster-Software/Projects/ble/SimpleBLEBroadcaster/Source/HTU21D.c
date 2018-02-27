// Board-Config
#include "OnBoard.h"

// Application
#include "HTU21D.h"

// Drivers
#include "hal_i2c.h"

uint8 init_HTU21D (void) {  
  HalI2CInit (i2cClock_267KHZ);
  
  uint8 ACK = 0;
  uint8 writeBuffer [2];
  
  writeBuffer[0] = 0xE6;        // User Configuration
  writeBuffer[1] = 0x03;        // Humidity 8 Bit, Temperature 12 Bit, OTP disabled
  ACK = HalI2CWrite (HTU21D_I2C_ADDRESS, 2, writeBuffer);
  
  HalI2CEnterSleep ();
  
  if (ACK == 0) return 0;
  else return 1;
}

uint8 measure_temperature_HTU21D (void) {
  HalI2CExitSleep ();
  
  uint8 ACK = 0;
  uint8 writeBuffer [1];
  
  // Start measure Temperature
  writeBuffer[0] = 0xF3;
  ACK = HalI2CWrite (HTU21D_I2C_ADDRESS, 1, writeBuffer);
  
  HalI2CEnterSleep ();
  
  if (ACK == 0) return 0;
  else return 1;
}

uint8 measure_humidity_HTU21D (void) {
  HalI2CExitSleep ();
  
  uint8 ACK = 0;
  uint8 writeBuffer [1];
  
  // Start measure Humidity
  writeBuffer[0] = 0xF5;
  ACK = HalI2CWrite (HTU21D_I2C_ADDRESS, 1, writeBuffer);
  
  HalI2CEnterSleep ();
  
  if (ACK == 0) return 0;
  else return 1;
}

uint8 read_temperature_HTU21D (int16 *temp) {
  HalI2CExitSleep ();
  
  uint8 readBuffer [2];
  
  // Read Temperature
  HalI2CRead ( HTU21D_I2C_ADDRESS, 2, readBuffer );
  uint16 tempCode = ( readBuffer[1] & 252) | (readBuffer[0] << 8 );     // Merge MSB and LSB
  float temperature = ( 175.72 * tempCode / 65536 ) - 46.85;            // Convert into temperature
  *temp = (int16)(temperature * 100);                                   // Convert into m°C
  
  HalI2CEnterSleep ();
  return 1;
}

uint8 read_humidity_HTU21D (uint16 *hum) {
  HalI2CExitSleep ();
  
  uint8 readBuffer [2];
  
  // Read Humidity
  HalI2CRead ( HTU21D_I2C_ADDRESS, 2, readBuffer );
  uint16 humCode = (readBuffer[1] & 252) | (readBuffer[0] << 8);        // Merge MSB and LSB
  float humidity = (((float)humCode / 65536) * 125) - 6;                // Convert into humidity
  *hum = (uint16)(humidity * 100);
  
  // Soft reset
  soft_reset_HTU21D ();
  
  HalI2CEnterSleep ();
  return 1;
}

uint8 soft_reset_HTU21D (void) {
  uint8 ACK = 0;
  uint8 writeBuffer [1];
  
  // Soft Reset
  writeBuffer[0] = 0xFE;
  ACK = HalI2CWrite (HTU21D_I2C_ADDRESS, 1, writeBuffer);
  
  if (ACK == 0) return 0;
  else return 1;
}