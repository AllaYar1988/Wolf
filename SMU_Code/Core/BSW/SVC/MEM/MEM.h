/*******************
 ***************************************** C SOURCE FILE ******************************************
 *******************/
/**
 *  @file       MEM.C.H
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
 *  @par        This file create for Memory Service
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
 *  @li @b      Date            : 2025-05-30
 *  @li @b      Author          : A.Moazami
 *  @li @b      Approved @b by  :
 *  @li @b      Description
 *
 *              Initial Version
 *
 **************************************************************************************************
 */

#ifndef _MEM_H_
#define _MEM_H_

#include "Platform.h"


#define GET_SD_DETECT (HAL_GPIO_ReadPin(SD_Detect_GPIO_Port,SD_Detect_Pin)) ? 1 : 0 /* Get CTS Status */


// Define the maximum buffer size
#define MAX_BUFFER_SIZE 100
//SDCard
typedef enum {
	MEM_INIT,
	MEM_IDLE,
	MEM_SAVE_CONFIG,
	MEM_READ_CONFIG,
	MEM_DISK_INIT,
	MEM_WAIT
}MEM_STATES;
typedef struct{
	uint32_t logTime;
}SD_HAndle_Type;

/*!
 **************************************************************************************************
 *
 *  @fn         void memHnadler(int index,MEM_STATES state);
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
u8 memHnadler(u16 index,MEM_STATES state);
/*!
 **************************************************************************************************
 *
 *  @fn         void memInit();
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
void memInit();
/*!
 **************************************************************************************************
 *
 *  @fn         u8 MEM_SdStatusCheck(void);
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
u8 MEM_SdStatusCheck(void);
/*!
 **************************************************************************************************
 *
 *  @fn         int SD_init(char immidiate)
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
int SD_init(char immidiate);

#endif /* BSW_SVC_MEM_MEM_H_ */
