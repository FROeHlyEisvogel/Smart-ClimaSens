///////////////////////////////
//--------- Defines ---------//
///////////////////////////////

// Active component selection
#define ADVERTISE               TRUE
#define INTERNAL_VOLTAGE        TRUE
#define INTERNAL_TEMPERATURE    FALSE
#define LUMINANCE_SENSOR        TRUE
#define Si7021                  TRUE
#define BME280                  TRUE
#define HTU21D                  TRUE
#define CONTACT_INTERRUPT       FALSE

// Time defines in ms
#define INIT_DELAY              1000
#define INTERVALL               1000 * 20
#define LUMINANCE_DELAY1        25
#define LUMINANCE_DELAY2        1000
#define HTU21D_DELAY            15
#define Si7021_DELAY            15
#define BME280_DELAY            15

#define INTERNAL_VOLTAGE_INTERVALL_SECONDS      60
#define INTERNAL_VOLTAGE_INTERVALL              (INTERNAL_VOLTAGE_INTERVALL_SECONDS / (INTERVALL / 1000))

#define LUMINANCE_SENSOR_INTERVALL_SECONDS      1
#define LUMINANCE_SENSOR_INTERVALL              (LUMINANCE_SENSOR_INTERVALL_SECONDS / (INTERVALL / 1000))

#define Si7021_INTERVALL_SECONDS                60
#define Si7021_INTERVALL                        (Si7021_INTERVALL_SECONDS / (INTERVALL / 1000))

#define BME280_INTERVALL_SECONDS                60
#define BME280_INTERVALL                        (BME280_INTERVALL_SECONDS / (INTERVALL / 1000))

#define HTU21D_INTERVALL_SECONDS                1
#define HTU21D_INTERVALL                        (HTU21D_INTERVALL_SECONDS / (INTERVALL / 1000))

#define REF_125V_CONVERSION     0.61065         // Conversion constant for 1.24V Reference
#define TEMP_COEFF              2.43            // Temperaturkoeffitient
#define TEMP_OFFSET             (750 + 50)      // Datasheet

///////////////////////////////
//------ Include Files ------//
///////////////////////////////

// Board-Config
#include "OnBoard.h"

// Application
#include "Application.h"
#include "BME280.h"
#include "Si7021.h"
#include "HTU21D.h"

// Drivers
#include "hal_adc.h"
#include "hal_i2c.c"

// BLE
#include "simpleBLEBroadcaster.h"

// Global variables
static uint8 Application_TaskID;   // Task ID for internal task/event processing
static uint8 Si7021_init_state = 0;
static uint8 BME280_init_state = 0;
static uint8 HTU21D_init_state = 0;

///////////////////////////////
//-------- Functions --------//
///////////////////////////////

void Application_Init (uint8 task_id) {
  Application_TaskID = task_id;
  
  init_SupplySensor ();
  
  init_adc ();
  
  osal_start_timerEx ( Application_TaskID, START_EVENT, INIT_DELAY );
}

void init_Modules (void) {
  
#if LUMINANCE_SENSOR == TRUE
  init_luminance_sensor ();
#endif
  
#if Si7021 == TRUE
  Si7021_init_state = init_Si7021 ();
#endif
  
#if BME280 == TRUE
  BME280_init_state = init_BME280 ();
#endif
  
#if HTU21D == TRUE
  if (Si7021_init_state == 0 ) {
    HTU21D_init_state = init_HTU21D ();
  }
#endif
  
  Advertise_init ();
  
  init_Contact ();
}

uint16 Application_ProcessEvent (uint8 task_id, uint16 events) {
  VOID task_id; // OSAL required parameter that isn't used in this function
  
  if ( events & SYS_EVENT_MSG ) {
    uint8 *pMsg;
  
    if ( (pMsg = osal_msg_receive( Application_TaskID )) != NULL ) {
      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }
  
    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }
  
  if ( events & START_EVENT ) {
    init_Modules ();
    osal_start_reload_timer ( Application_TaskID, INTERVALL_EVENT, INTERVALL );
    osal_set_event( Application_TaskID, INTERVALL_EVENT );
    return ( events ^ START_EVENT);
  }
  
  if ( events & INTERVALL_EVENT ) {
    
#if INTERNAL_TEMPERATURE == TRUE
    AdverisingUpdate_Internal_Temperature (read_internal_temperature ());
#endif
    
#if LUMINANCE_SENSOR == TRUE
    static uint16 luminance_sensor_intervall_counter = 0xFFFF;
    if (luminance_sensor_intervall_counter >= LUMINANCE_SENSOR_INTERVALL) {
      start_luminance_sensor ();
      luminance_sensor_intervall_counter = 0;
    }
    else luminance_sensor_intervall_counter += 1;
#endif
    
#if Si7021 == TRUE
    if (Si7021_init_state == 1) {
      static uint16 Si7021_intervall_counter = 0xFFFF;
      if (Si7021_intervall_counter >= Si7021_INTERVALL) {
        if (start_Si7021 () == 1) osal_start_timerEx ( Application_TaskID, Si7021_EVENT, Si7021_DELAY);
        Si7021_intervall_counter = 0;
      }
      else Si7021_intervall_counter += 1;
    }
#endif

#if BME280 == TRUE
    if (BME280_init_state == 1) {
      static uint16 BME280_intervall_counter = 0xFFFF;
      if (BME280_intervall_counter >= BME280_INTERVALL) {
        if (start_BME280 () == 1) osal_start_timerEx ( Application_TaskID, BME280_EVENT, BME280_DELAY );
        BME280_intervall_counter = 0;
      }
      else BME280_intervall_counter += 1;
    }
#endif

#if HTU21D == TRUE
    if (HTU21D_init_state == 1) {
      static uint16 HTU21D_intervall_counter = 0xFFFF;
      if (HTU21D_intervall_counter >= HTU21D_INTERVALL) {
        if (measure_temperature_HTU21D () == 1) osal_start_timerEx ( Application_TaskID, HTU21D_TEMP_EVENT, HTU21D_DELAY);
        else AdverisingUpdate_Temperature (0xFFFF);
        HTU21D_intervall_counter = 0;
      }
      else HTU21D_intervall_counter += 1;
    }
#endif
    
    AdverisingUpdate_Contact (read_Contact());
    
#if INTERNAL_VOLTAGE == TRUE
    static uint16 internal_voltage_intervall_counter = 0xFFFF;
    if (internal_voltage_intervall_counter >= INTERNAL_VOLTAGE_INTERVALL) {
      AdverisingUpdate_Internal_Voltage (read_internal_voltage ());
      internal_voltage_intervall_counter = 0;
    }
    else internal_voltage_intervall_counter += 1;
#endif
    
#if ADVERTISE == TRUE
    Advertise_single ();
#endif
    
    return ( events ^ INTERVALL_EVENT);
  }
  
  if ( events & LUMINANCE_EVENT ) {
    read_luminance_sensor ();
    return ( events ^ LUMINANCE_EVENT);
  }
  
  if ( events & Si7021_EVENT ) {
    int16 temp = 0;
    uint16 hum  = 0;
    if (read_Si7021 (&temp, &hum) == 1) {
      AdverisingUpdate_Temperature (temp);
      AdverisingUpdate_Humidity (hum);
    }
    return ( events ^ Si7021_EVENT);
  }
  
  if ( events & BME280_EVENT ) {
    int16 temp  = 0;
    uint16 hum   = 0;
    uint16 press = 0;
    if (read_BME280 (&temp, &hum, &press) == 1) {
      AdverisingUpdate_Temperature (temp);
      AdverisingUpdate_Humidity (hum);
      AdverisingUpdate_Barometric (press);
    }
    return ( events ^ BME280_EVENT);
  }
  
  if ( events & HTU21D_TEMP_EVENT ) {
    int16 temp = 0;
    if (read_temperature_HTU21D (&temp) == 1) {
      AdverisingUpdate_Temperature (temp);
      if (measure_humidity_HTU21D () == 1) osal_start_timerEx ( Application_TaskID, HTU21D_HUM_EVENT, HTU21D_DELAY);
      else AdverisingUpdate_Humidity (0xFFFF);
    }
    return ( events ^ HTU21D_TEMP_EVENT);
  }
  
  if ( events & HTU21D_HUM_EVENT ) {
    uint16 hum = 0;
    if (read_humidity_HTU21D (&hum) == 1) {
      AdverisingUpdate_Humidity (hum);
    }
    return ( events ^ HTU21D_HUM_EVENT);
  }
  
  return 0;
}

void init_adc (void) {
  // Initialize ADC
  HalAdcInit ();
  HalAdcSetReference (HAL_ADC_REF_125V);
}

uint16 read_internal_voltage (void) {
  uint16 ADC_Operating_Voltage = HalAdcRead (HAL_ADC_CHANNEL_VDD, HAL_ADC_RESOLUTION_12);
  uint16 Operating_Voltage = (int)(ADC_Operating_Voltage * REF_125V_CONVERSION * 3);
  return Operating_Voltage;
}

uint8 read_internal_temperature (void) {
  ATEST = 1;    // Activate Temperature Sensor
  TR0 = 1;      // Connect Temperature Sensor to ADC
  
  uint16 ADC_Operating_Temperature = HalAdcRead (HAL_ADC_CHANNEL_TEMP, HAL_ADC_RESOLUTION_12);
  uint16 Operating_Temperature = (int)(ADC_Operating_Temperature * REF_125V_CONVERSION);
  Operating_Temperature = (int)((Operating_Temperature - TEMP_OFFSET) / TEMP_COEFF);
  ATEST = 0;    // Deaktivate Temperature Sensor
  return Operating_Temperature;
}

void init_luminance_sensor (void) { 
  // Initialize P0_0 for LED+
  P0SEL &= ~1;            // Function as General Purpose I/O.
  P0_0 = 0;               // GND
  P0DIR |= 1;             // Output
  P0INP |= 1;             // Disable pullup / pulldown
  
  // Initialize P0_1 for LED-
  P0SEL &= ~2;            // Function as General Purpose I/O.
  P0_1 = 0;               // GND
  P0DIR |= 2;             // Output
  P0INP |= 2;             // Disable pullup / pulldown
}

static uint16 luminanceDelay = LUMINANCE_DELAY1;
void start_luminance_sensor (void) {
  P0DIR &= ~1;           // Input
  APCFG |= 1;            // configurate PIN0_0 as ADC-Input for luminance sensor
  osal_start_timerEx ( Application_TaskID, LUMINANCE_EVENT, luminanceDelay);
}

void read_luminance_sensor (void) {
  uint16 ADC_Luminance = HalAdcRead (HAL_ADC_CHANNEL_0, HAL_ADC_RESOLUTION_12);
  uint16 Luminance = ADC_Luminance;
  
  if (Luminance > 0x07F0 && luminanceDelay == LUMINANCE_DELAY2) {
    luminanceDelay = LUMINANCE_DELAY1;
  }
  else if (Luminance < 0x000F && luminanceDelay == LUMINANCE_DELAY1) {
    luminanceDelay = LUMINANCE_DELAY2;
  }
  else {
    if (luminanceDelay == LUMINANCE_DELAY1) Luminance += 0x07FF;
    AdverisingUpdate_Luminance ( Luminance );
  }
  
  P0SEL &= ~1;          // Function as General Purpose I/O.
  P0DIR |= 1;           // Output
  P0_0 = 0;             // GND
}

void init_SupplySensor (void) {
  // Initialize P1_0 for VCC
  P1SEL &= ~1;            // Function as General Purpose I/O.
  P1_0 = 1;               // VCC
  P1DIR |= 1;             // Output
  P1INP |= 1;             // Disable pullup / pulldown
  
  // Initialize P1_4 for GND
  P1SEL &= ~16;           // Function as General Purpose I/O.
  P1_4 = 0;               // GND
  P1DIR |= 16;            // Output
  P1INP |= 16;            // Disable pullup / pulldow
}

void init_Contact (void) {
  // Initialize P1_1 for VCC
  P1SEL &= ~2;            // Function as General Purpose I/O.
  P1_1 = 1;               // VCC
  P1DIR |= 2;             // Output
  P1INP |= 2;             // Disable pullup / pulldown
  
  // Initialize P1_2 as Input
  P1SEL &= ~4;            // Function as General Purpose I/O.
  P1_2 = 0;               // GND
  P1DIR &= ~4;            // Input
  P1INP |= 4;             // Disable pullup / pulldown

  P2INP |= 64;            // Port1 Pulldown
  
#if CONTACT_INTERRUPT == TRUE
  // Initialize P1_2 as Interrupt
  P1IEN |= 4;             // Interrupt Mask
  PICTL &= ~2;            // Interrupt on rising Edge
  P1IFG &= ~4;            // Clear Interrupt
  IEN2 |= 16;             // Enable Interrupts on Port 1
#endif
}

HAL_ISR_FUNCTION( halKeyPort1Isr, P1INT_VECTOR ) {
  HAL_ENTER_ISR();
  
  if (read_Contact() == 1) {
    AdverisingUpdate_Contact (1);
    PICTL |= 2;           // Interrupt on falling Edge
  }
  else {
    AdverisingUpdate_Contact (0);
    PICTL &= ~2;          // Interrupt on rising Edge
  }
  
  Advertise_single ();

  P1IFG &= ~4;            // Clear Interrupt
  P1IF   = 0;             // Clear Port Interrupt
  
  CLEAR_SLEEP_MODE();
  HAL_EXIT_ISR();
  
  return;
}

uint8 read_Contact (void) {
  P1INP &= ~4;             // Enable pullup / pulldown
  uint8 temp_Contact = P1_2;
  P1INP |= 4;              // Disable pullup / pulldown
  return temp_Contact;
}