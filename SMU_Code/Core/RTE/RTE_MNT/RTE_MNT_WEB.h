
/*******************
 ***************************************** C SOURCE FILE ******************************************
 *******************/
/**
 *  @file       RTE_MNT_WEB.h
 *
 *  General Info
 *  ------------
 *  ____
 *  @par        File info
 *  @li @b      Version         : 1.0.4
 *  @li @b      Date            : 2024-05-04
 *
 *  @par        Project info
 *  @li @b      Project         : xxxxx
 *  @li @b      Processor       : STM32f4xx
 *  @li @b      Tool  @b Chain  :
 *  @li @b      Clock @b Freq   : 168 MHz
 *
 *  @par        This file create for System Memory and Web Communication State Machine
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
 *  @li @b      Date            : 2024-05-04
 *  @li @b      Author          : A.Moazami
 *  @li @b      Approved @b by  :
 *  @li @b      Description
 *
 *              Initial Version
 *
 **************************************************************************************************
 */
#ifndef RTE_RTE_MNT_RTE_MNT_WEB_H_
#define RTE_RTE_MNT_RTE_MNT_WEB_H_
#include <stdint.h>



#define MEMORY_TICK 50

typedef enum{
	WEB_MNG_ENTRY=0,
	WEB_MNG_IDLE,
	WEB_MNG_SENDING,
	WEB_MNG_GET_MODEM_CLK,
	WEB_MNG_GET_SERVER_CLK

}WEB_MNG_Enu;
typedef enum{
	LOG_MNG_ENTRY=0,
	LOG_MNG_SAVE,
	LOG_MNG_IDLE
}LOG_MNG_Enu;

typedef enum{
	RTE_MNT_ENTRY=0,
	RTE_MNT_DO
}RTE_MNT_Enu;

typedef struct {
	WEB_MNG_Enu state;
	uint32_t tick;
}WEB_MNG_STU_Type;

typedef struct {
	uint16_t cDataInMem;
	uint16_t pTime;
	uint16_t vIndex;
	LOG_MNG_Enu state;
	uint32_t tick;
	uint32_t logtick;
}LOG_MNG_STU_Type;

/*!
 **************************************************************************************************
 *
 *  @fn         void RTE_MNT_MNG(void)
 *
 *  @par        This function is for RTE Monitoring State Manager
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
void RTE_MNT_MNG(void);


#endif /* RTE_RTE_MNT_RTE_MNT_WEB_H_ */
