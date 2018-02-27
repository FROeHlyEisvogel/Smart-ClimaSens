// Board-Config
#include "OnBoard.h"

// Application
#include "Si7021.h"

// Drivers
#include "hal_i2c.h"

uint8 init_Si7021 (void) {  
  HalI2CInit (i2cClock_267KHZ);
  
  uint8 ACK = 0;
  uint8 writeBuffer [2];
  
  writeBuffer[0] = 0xE6;        // User Configuration
  writeBuffer[1] = 0x01;        // Humidity 8 Bit, Temperature 12 Bit
  ACK = HalI2CWrite (Si7021_I2C_ADDRESS, 2, writeBuffer);
  
  ACK = read_SerialNumber_Si7021 ();
  
  HalI2CEnterSleep ();
  
  if (ACK == 0) return 0;
  else return 1;
}

uint8 read_SerialNumber_Si7021 (void) {
  uint8 ACK = 0;
  uint8 writeBuffer [2];
  uint8 readBuffer [4];
  
  writeBuffer[0] = 0xFC;        // SerialNumber 2. access
  writeBuffer[1] = 0xC9;        // SerialNumber 2. access
  ACK = HalI2CWrite (Si7021_I2C_ADDRESS, 2, writeBuffer);
  HalI2CRead ( Si7021_I2C_ADDRESS, 4, readBuffer );
  
  if (readBuffer[0] != Si7021_DEVICE_INDICATOR) ACK = 0;
  
  if (ACK == 0) return 0;
  else return 1;
}

uint8 start_Si7021 (void) {
  HalI2CExitSleep ();
  
  uint8 ACK = 0;
  uint8 writeBuffer [1];
  
  // Start measure Humidity
  writeBuffer[0] = 0xF5;
  ACK = HalI2CWrite (Si7021_I2C_ADDRESS, 1, writeBuffer);
  
  HalI2CEnterSleep ();
  
  if (ACK == 0) return 0;
  else return 1;
}

uint8 read_Si7021 (int16 *temp, uint16 *hum) {
  HalI2CExitSleep ();
  
  uint8 ACK = 0;
  uint8 readBuffer [2];
  uint8 writeBuffer [1];
  
  // Measure and Read Humidity
  HalI2CRead ( Si7021_I2C_ADDRESS, 2, readBuffer );
  uint16 humCode = (readBuffer[1] & 252) | (readBuffer[0] << 8);        // Merge MSB and LSB
  float humidity = (((float)humCode / 65536) * 125) - 6;                // Convert into humidity
  *hum = (uint16)(humidity * 100);                                      // Convert into m%
    
  // Read Temperature
  writeBuffer[0] = 0xE0;
  ACK = HalI2CWrite ( Si7021_I2C_ADDRESS, 1, writeBuffer );
  if (ACK == 0) return 0;
  
  HalI2CRead ( Si7021_I2C_ADDRESS, 2, readBuffer );
  uint16 tempCode = ( readBuffer[1] & 252) | (readBuffer[0] << 8 );     // Merge MSB and LSB
  float temperature = ( 175.72 * tempCode / 65536 ) - 46.85;            // Convert into temperature
  *temp = (int16)(temperature * 100);                                   // Convert into m°C
  
  HalI2CEnterSleep ();
  return 1;
}