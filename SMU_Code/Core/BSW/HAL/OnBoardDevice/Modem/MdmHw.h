                                    /*******************
 ***************************************** C SOURCE FILE ******************************************
                                        *******************/
/**
 *  @file       MdmHw.H
 *
 *  General Info
 *  ------------
 *  ____
 *  @par        File info
 *  @li @b      Version         : 1.0.4
 *  @li @b      Date            : 2024-05-20
 *
 *  @par        Project info
 *  @li @b      Project         : xxxxx
 *  @li @b      Processor       : STM32f4xx
 *  @li @b      Tool  @b Chain  :
 *  @li @b      Clock @b Freq   : 168 MHz
 *
 *  @par        This file create for use Media Access Control (MAC) for UART
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

#ifndef _MDMHW_H_
#define _MDMHW_H_

#define ON GPIO_PIN_SET
#define OFF GPIO_PIN_RESET

#define 	MODEM_POWER(x) HAL_GPIO_WritePin(uControl3V9_GPIO_Port,uControl3V9_Pin,x);

#endif /* BSW_HAL_ONBOARDDEVICE_MODEM_MDMHW_H_ */
