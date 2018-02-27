/**************************************************************************************************
 *                                           Includes
 **************************************************************************************************/
/* Hal Drivers */
#include "hal_timer.h"
#include "hal_drivers.h"

/* OSAL */
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OSAL_PwrMgr.h"
#include "osal_snv.h"
#include "OnBoard.h"

/* BLE */
#include "simpleBLEBroadcaster.h"
#include "Application.h"

/**************************************************************************************************
 * FUNCTIONS
 **************************************************************************************************/

/**************************************************************************************************
 * @fn          main
 *
 * @brief       Start of application.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
int main(void)
{
  /* Initialize hardware */
  HAL_BOARD_INIT();
  
  // Interrupts off
  osal_int_disable( INTS_ALL );
 
  /* Initialze the HAL driver */
  HalDriverInit();
  
  /* Initialize NV system */
  osal_snv_init();
  
  /* Initialize LL */
  
  /* Initialize the operating system */
  osal_init_system();
  
  /* Enable interrupts */
  HAL_ENABLE_INTERRUPTS();
  
  #if defined ( POWER_SAVING )
    osal_pwrmgr_device( PWRMGR_BATTERY );
  #endif
  
  /* Start OSAL */
  osal_start_system(); // No Return from here
  
  return 0;
}

/**************************************************************************************************
                                           CALL-BACKS
**************************************************************************************************/


/*************************************************************************************************
**************************************************************************************************/
