
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

/*
*********************************************************************************************************
*
*                                            MODULES USED
*
*********************************************************************************************************
*/
#include "SchCore.h"
#include "Platform.h"



/*
*********************************************************************************************************
*
*                                        DEFINITIONS AND MACROS
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           PUBLIC VARIABLES
*
*********************************************************************************************************
*/

volatile StructSchTask_t SCC_AStructSchTasks[SCH_MAX_TASKS];

/**
*	Used to display the error code
*	See SchCore.h for details of error codes
*/
u8 SCC_Int8uErrorCode = 0u;




/*
*********************************************************************************************************
*
*                                           PRIVATE VARIABLES
*
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*
*                                     PROTOTYPES OF PRIVATE FUNCTIONS
*
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*
*                                     SOURCES OF PUBLIC FUNCTIONS
*
*********************************************************************************************************
*/

/**
*********************************************************************************************************
*	\fn void SCC_VoidDispatchTasks(void)
*
*	\breif
*	This is the 'dispatcher' function.  When a task (function)
*	is due to run, SCC_VoidDispatchTasks() will run it.
*	This function must be called (repeatedly) from the main loop.
*
*********************************************************************************************************
*/
void SCC_VoidDispatchTasks(void)
{
	u8 int8uIndex = 0u;

			/* Run the task */
			(*SCC_AStructSchTasks[int8uIndex].pTask)();


}

/**
*********************************************************************************************************
*	\fn int8u SCC_Int8uAddTask(PVoidCallBackFunc_t pCallBackFunc,
*                   	   		int16u   int16uDelay,
*                        		int16u   int16uPeriod)
*
*********************************************************************************************************
*/
u8 SCC_Int8uAddTask( const PVoidCallBackFuncType pCallBackFunc,

		u16   int16uDelay,
                        u16   int16uPeriod)
{
   u8 int8uIndex = 0u;
   u8 int8uReturnValue = SCH_MAX_TASKS;
   bool  boolExitFlag = false;

	while ((int8uIndex < SCH_MAX_TASKS) && (false == boolExitFlag))
	{
		if (SCC_AStructSchTasks[int8uIndex].pTask != 0)
		{
			int8uIndex++;
		}
		else
		{
			boolExitFlag = true;
		}
	}


   /* Have we reached the end of the list? */
   if (int8uIndex == SCH_MAX_TASKS)
   {
      /* Task list is full
       	Set the global error variable */
      SCC_Int8uErrorCode = ERROR_SCH_TOO_MANY_TASKS;

      /* Also return an error code */
      int8uReturnValue = SCH_MAX_TASKS;
   }
	else
	{
		/* If we're here, there is a space in the task array */
		SCC_AStructSchTasks[int8uIndex].pTask = pCallBackFunc;

		SCC_AStructSchTasks[int8uIndex].int16uDelay  = int16uDelay;
		SCC_AStructSchTasks[int8uIndex].int16uPeriod = int16uPeriod;

		SCC_AStructSchTasks[int8uIndex].int8uRunMe  = 0u;

		SCC_AStructSchTasks[int8uIndex].int16uET  = 0u;

		/* return position of task (to allow later deletion) */
		int8uReturnValue = int8uIndex;

	}

 	/* Exit from function with int8uReturnValue */
	return int8uReturnValue;

}

/**
*********************************************************************************************************
*	\fn int8u SCC_Int8uDeleteTask(int8u int8uTaskIndex)
*
*	\breif
*	Removes a task from the scheduler.  Note that this does
*  not delete the associated function from memory:
*  it simply means that it is no longer called by the scheduler.
*
*	\param int8uTaskIndex
*	The task index.  Provided by SCC_Int8uAddTask().
*
*	\return
*	RETURN_ERROR or RETURN_NORMAL
*
*********************************************************************************************************
*/
u8 SCC_Int8uDeleteTask(u8 int8uTaskIndex)
{
	u8 int8uReturnValue = RETURN_NORMAL;

	if (SCC_AStructSchTasks[int8uTaskIndex].pTask == 0)
	{
		/* No task at this location...
			Set the global error variable */
		SCC_Int8uErrorCode = ERROR_SCH_CANNOT_DELETE_TASK;

		/* ...also return an error code */
		int8uReturnValue = RETURN_ERROR;
	}

	SCC_AStructSchTasks[int8uTaskIndex].pTask   = (void*)0;
	SCC_AStructSchTasks[int8uTaskIndex].int16uDelay   = 0u;
	SCC_AStructSchTasks[int8uTaskIndex].int16uPeriod  = 0u;

	SCC_AStructSchTasks[int8uTaskIndex].int8uRunMe   = 0u;

	SCC_AStructSchTasks[int8uTaskIndex].int16uET   = 0u;

	/* Exit from function with int8uReturnValue */
	return int8uReturnValue;
}

/*
*********************************************************************************************************
*
*                                              END OF FILE
*
*********************************************************************************************************
*/
