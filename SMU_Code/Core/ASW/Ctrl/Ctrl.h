                                    /*******************
 ***************************************** C SOURCE FILE ******************************************
                                        *******************/
/**
 *  @file       CTRL.H
 *
 *  General Info
 *  ------------
 *  ____
 *  @par        File info
 *  @li @b      Version         : 1.0.4
 *  @li @b      Date            : 2024-05-14
 *
 *  @par        Project info
 *  @li @b      Project         : xxxxx
 *  @li @b      Processor       : STM32f4xx
 *  @li @b      Tool  @b Chain  :
 *  @li @b      Clock @b Freq   : 168 MHz
 *
 *  @par        This file create for Control Algorithms for SMU
 *
 *  @copyright  (C)
 *
 **************************************************************************************************
 *  _______________
 *  Version History
 *  ---------------
  **************************************************************************************************
 *
 *              This veraion has been tested.
 *  ____
 *  @par        Rev 1.0.0
 *  @li @b      Date            : 2025-05-04
 *  @li @b      Author          : A.Moazami
 *  @li @b      Approved @b by  :
 *  @li @b      Description
 *
 *              Initial Version
 *
 **************************************************************************************************
 */
#ifndef ASW_CTRL_CTRL_H_
#define ASW_CTRL_CTRL_H_
#include "Platform.h"
#include "tempSensor.h"


#define SSR_ON          HAL_GPIO_ReadPin(uDi1_GPIO_Port,uDi1_Pin)
#define GEN_ON          HAL_GPIO_ReadPin(uDi2_GPIO_Port,uDi2_Pin)

#define RelayOn 0
#define RelayOff 1

#define OptoOn 0
#define OptoOff 1



// Define LED states
#define LED_ON  0
#define LED_OFF 1

#define BATT_POWER_LOW_HYST -200
#define BATT_POWER_HIGH_HYST 200
#define BATT_STATE_IDLE         1U
#define BATT_STATE_CHARGING     2U
#define BATT_STATE_DISCHARGING  3U
#define BATT_STATE_DEAD_RCOVERY 4U
#define BATTERY_DEAD            4U


// RGB LED color options
typedef enum {
    LED_SOLID_OFF,
    LED_SOLID_RED,
    LED_SOLID_GREEN,
    LED_SOLID_BLUE,
	LED_BLINK_RED,
	LED_BLINK_GREEN,
	LED_BLINK_BLUE
} LED_Color_t;

extern LED_Color_t LED_COLOR;
/*!
 **************************************************************************************************
 *
 *  @fn         void Ctrl_Handler(void)
 *
 *  @par
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
void Ctrl_Handler(void);
/*!
 **************************************************************************************************
 *
 *  @fn         void Ctrl_GeneralControl(void)
 *
 *  @par
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
void Ctrl_GeneralControl(void);


#endif /* ASW_CTRL_CTRL_H_ */
