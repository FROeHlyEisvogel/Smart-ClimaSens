/*********************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"

#include "hci.h"
#include "gap.h"

#include "devinfoservice.h"
#include "broadcaster.h"

#include "simpleBLEBroadcaster.h"

#include "ll_sleep.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// What is the advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          (1600 * 10)

// Company Identifier: Texas Instruments Inc. (13)
#define TI_COMPANY_ID                         0x000D

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 simpleBLEBroadcaster_TaskID;   // Task ID for internal task/event processing

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
{
  // complete name
  0x0E,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  'C',
  'l',
  'i',
  'm',
  'a',
  't',
  'e',
  'S',
  'e',
  'n',
  's',
  'o',
  'r',

  // Tx power level
  0x02,   // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  0       // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[] =
{ 
  // Flags; this sets the device to use limited discoverable
  // mode (advertises for 30 seconds at a time) instead of general
  // discoverable mode (advertises indefinitely)
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
  
  0x10,   // length of this data including the data type byte
  GAP_ADTYPE_MANUFACTURER_SPECIFIC,      // manufacturer specific advertisement data type
  13,   // MSB Company ID
  0,    // LSB Company ID
  0,    // MSB Internal Voltage
  0,    // LSB Internal Voltage
  0,    // MSB Internal Temperature
  0,    // LSB Internal Temperature
  0,    // MSB Luminance
  0,    // LSB Luminance
  0,    // MSB Temperature
  0,    // LSB Temperature
  0,    // MSB Humidity
  0,    // LSB Humidity
  0,    // MSB Barometirc
  0,    // LSB Barometirc
  0     // Open/Close
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void peripheralStateNotificationCB( gaprole_States_t newState );

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t simpleBLEBroadcaster_BroadcasterCBs =
{
  peripheralStateNotificationCB,  // Profile State Change Callbacks
  NULL                            // When a valid RSSI is read from controller (not used by application)
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleBLEBroadcaster_Init
 *
 * @brief   Initialization function for the Simple BLE Broadcaster App
 *          Task. This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SimpleBLEBroadcaster_Init( uint8 task_id )
{
  simpleBLEBroadcaster_TaskID = task_id;

  // Setup the GAP Broadcaster Role Profile
  {
    // For other hardware platforms, device starts advertising upon initialization
    uint8 initial_advertising_enable = TRUE;

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;
      
    //uint8 advType = GAP_ADTYPE_ADV_NONCONN_IND;   // use non-connectable advertisements
    uint8 advType = GAP_ADTYPE_ADV_SCAN_IND; // use scannable unidirected advertisements

    // Set the GAP Role Parameters
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );
    
    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );
    
    GAPRole_SetParameter( GAPROLE_ADV_EVENT_TYPE, sizeof( uint8 ), &advType );
    
    //GAP_SetParamValue( TGAP_LIM_ADV_TIMEOUT, 1);        // only advertise once
    //GAP_SetParamValue( TGAP_GEN_DISC_ADV_MIN, 1);       // only advertise once
  }

  // Set advertising interval
  {
    uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;
    
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt ); // 32-16384
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
  }

  
  // Setup a delayed profile startup
  osal_set_event( simpleBLEBroadcaster_TaskID, SBP_START_DEVICE_EVT );
}

/*********************************************************************
 * @fn      SimpleBLEBroadcaster_ProcessEvent
 *
 * @brief   Simple BLE Broadcaster Application Task event processor. This
 *          function is called to process all events for the task. Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 SimpleBLEBroadcaster_ProcessEvent( uint8 task_id, uint16 events )
{
  VOID task_id; // OSAL required parameter that isn't used in this function
  
  if ( events & SYS_EVENT_MSG ) {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( simpleBLEBroadcaster_TaskID )) != NULL ) {
      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & SBP_START_DEVICE_EVT ) {
    // Start the Device
    VOID GAPRole_StartDevice( &simpleBLEBroadcaster_BroadcasterCBs );
    
    return ( events ^ SBP_START_DEVICE_EVT );
  }
  
  if ( events & SBP_STOP_ADVERTISE ) {
    // Stop the Device
    LL_SetAdvControl (LL_ADV_MODE_OFF);
    
    osal_stop_timerEx ( 0 , 0 );
    osal_start_timerEx ( 0 , 0, 60000 );
    
    //LL_Reset ();
    
    return ( events ^ SBP_STOP_ADVERTISE );
  }
  
  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB( gaprole_States_t newState ) {
  switch ( newState )
  {
    case GAPROLE_STARTED: {
        uint8 ownAddress[B_ADDR_LEN];
        
        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);
      }
      break;
      
    case GAPROLE_ADVERTISING: {
        //P1_0 = 0;
      }
      break;

    case GAPROLE_WAITING: {
        //P1_0 = 0;
      }
      break;

    case GAPROLE_ERROR: {
        //P1_0 = 0;
      }
      break;
      
    default: {
        //P1_0 = 0;
      }
      break;
  }
}

void Advertise_init (void) {
  LL_SetScanRspData (sizeof ( scanRspData ), scanRspData);
  LL_SetAdvData (sizeof( advertData ), advertData);
  LL_SetAdvParam (160, 160, LL_ADV_SCANNABLE_UNDIRECTED_EVT, LL_DEV_ADDR_TYPE_PUBLIC, 0, NULL, LL_ADV_CHAN_ALL, LL_ADV_WL_POLICY_ANY_REQ);
}

void Advertise_single (void) {
  LL_SetAdvData (sizeof( advertData ), advertData);
  
  LL_SetAdvControl (LL_ADV_MODE_ON);
  osal_start_timerEx ( simpleBLEBroadcaster_TaskID, SBP_STOP_ADVERTISE, 4 );
}

void AdverisingUpdate_Internal_Voltage (uint16 Voltage) {
  advertData[7] = (Voltage >> 8);
  advertData[8] = Voltage;
}

void AdverisingUpdate_Internal_Temperature (uint16 Temperature) {
  advertData[9] = (Temperature >> 8);
  advertData[10] = Temperature;
}

void AdverisingUpdate_Luminance (uint16 Luminance) {
  advertData[11] = (Luminance >> 8);
  advertData[12] = Luminance;
}

void AdverisingUpdate_Temperature (int16 Temperature) {
  advertData[13] = (Temperature >> 8);
  advertData[14] = Temperature;
}

void AdverisingUpdate_Humidity (uint16 Humidity) {
  advertData[15] = (Humidity >> 8);
  advertData[16] = Humidity;
}

void AdverisingUpdate_Barometric (uint16 Barometric) {
  advertData[17] = (Barometric >> 8);
  advertData[18] = Barometric;
}

void AdverisingUpdate_Contact (uint8 Contact) {
  advertData[19] = Contact;
}