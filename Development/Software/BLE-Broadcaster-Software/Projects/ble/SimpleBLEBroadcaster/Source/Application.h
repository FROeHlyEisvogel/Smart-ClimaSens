#ifndef APPLICATION_H
#define APPLICATION_H

// Simple BLE Broadcaster Task Events
#define START_EVENT             0x0001
#define INTERVALL_EVENT         0x0002
#define LUMINANCE_EVENT         0x0004
#define Si7021_EVENT            0x0008
#define BME280_EVENT            0x0010
#define HTU21D_TEMP_EVENT       0x0020
#define HTU21D_HUM_EVENT        0x0040

#define HAL_I2C                 TRUE
#define HAL_I2C_MASTER          TRUE

void Application_Init (uint8 task_id);
uint16 Application_ProcessEvent (uint8 task_id, uint16 events);

void init_Modules (void);

uint16 read_internal_voltage (void);
uint8 read_internal_temperature (void);

void init_luminance_sensor (void);
void start_luminance_sensor (void);
void read_luminance_sensor (void);

void init_adc (void);
void init_SupplySensor (void);
void init_Contact (void);
uint8 read_Contact (void);

#endif /* APPLICATION_H */
