
                                        /*******************
***************************************** C SOURCE FILE ******************************************
                                        *******************/
/**
*  @file       SchCore.c/.h
*
*  General Info
*  ------------
*  ____
*  @par        File info
*  @li @b      Version         : 1.0.0
*  @li @b      Date            :
*
*  @par        Project info
*  @li @b      Project         : SMU
*  @li @b      Processor       : STM32f407
*  @li @b      Tool  @b Chain  : CUBE IDE
*  @li @b      Clock @b Freq   : 168 MHZ
*
*  @par        Description
*              This file is generated in order to meet the requirements of ACU operating state module.
*
*  @copyright
*
**************************************************************************************************
*  _______________
*  Version History
*  ---------------
**************************************************************************************************
*  ____
*  @par        Rev 1.0.0
*  @li @b      Date            : 5/14/2025
*  @li @b      Author          : Allahyar Moazami
*  @li @b      Approved @b by  :
*  @li @b      Description
*
*              Revision Tag : Enter revision tag related to the current revision.
*              Enter a paragraph that serves as a detail description.
*
**************************************************************************************************
*/

#ifndef _SCHCORE_H
#define _SCHCORE_H

/*
*********************************************************************************************************
*
*                                        DEFINITIONS AND MACROS
*
*********************************************************************************************************
*/
#include "Platform.h"
/* Error codes */
#define ERROR_SCH_TOO_MANY_TASKS 								(1U)
#define ERROR_SCH_CANNOT_DELETE_TASK 							(2U)

#define RETURN_ERROR 												(6U)
#define RETURN_NORMAL 												(7U)


/**	The maximum number of tasks required at any one time
		during the execution of the program
 		MUST BE ADJUSTED FOR EACH NEW PROJECT	*/
#define SCH_MAX_TASKS   											(1U)

/*
*********************************************************************************************************
*
*                                        TYPEDEFS AND STRUCTURES
*
*********************************************************************************************************
*/

/**
	\struct StructSchTask_t
	\brief
	Store in DATA area, if possible, for rapid access
	Total memory per task is 8 bytes
*/
typedef struct
{
	/**	Pointer to the task (must be a 'void (void)' function) */
	PVoidCallBackFuncType pTask;

	/**	int16uDelay (ticks) until the function will (next) be run.
			see SCH_Add_Task() for further details */
	u16 int16uDelay;

	/**	Interval (ticks) between subsequent runs.
			see SCH_Add_Task() for further details */
	u16 int16uPeriod;

	/** The execution time of specefic task (unit = us) **/
	u16 int16uET;

	/**	Incremented (by scheduler) when task is due to execute */
	u8 int8uRunMe;
}StructSchTask_t;


/*
*********************************************************************************************************
*
*                                           PUBLIC VARIABLES
*
*********************************************************************************************************
*/

/**	The array of tasks */
extern volatile StructSchTask_t SCC_AStructSchTasks[SCH_MAX_TASKS];

/**	Used to display the error code
		See SchCore.H for details of the error port */
extern u8 SCC_Int8uErrorCode;

/*
*********************************************************************************************************
*
*                                           PUBLIC FUNCTIONS
*
*********************************************************************************************************
*/
/** Core scheduler functions */
void SCC_VoidDispatchTasks(void);
u8 SCC_Int8uAddTask( const PVoidCallBackFuncType pCallBackFunc,
                   	   u16   int16uDelay,
                        u16   int16uPeriod);
u8 SCC_Int8uDeleteTask(u8 int8uTaskIndex);

#endif

/*
*********************************************************************************************************
*
*                                              END OF FILE
*
*********************************************************************************************************
*/
