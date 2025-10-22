                                        /*******************
***************************************** C SOURCE FILE ******************************************
                                        *******************/
/**
*  @file       RTE_Task1kHz.c/.h
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

#include "RTE_Task1kHz.h"
#include "RTE_Task500Hz_DT1.h"
#include "RTE_Task500Hz_DT0.h"

#include "Platform.h"
#include "SMU_MNG.h"
#include "MBM.h"
#include "MBS.h"
//#include "WCET.h"
/*
*********************************************************************************************************
*
*                                           PRIVATE VARIABLES
*
*********************************************************************************************************
*/
static void VoidFn1msInit(void);
static void VoidFn1msStartup(void);
static void VoidFn1msFullyOp(void);
static void VoidFn1msWakeup(void);
static void VoidFn1msFailSafe(void);
static void VoidFn1msEolUnlock(void);
static void VoidUpdateDelayTimesStatus(void);


EnuTask500HZStates_t	AP2_EnuReadyTask500HZ = TASK_500HZ_DT00;

/* The period of each call back function is 1ms. These functions will be call via Task250us respectively */
static const PVoidCallBackFuncEnuECUState_t ATask500Hz_DTxxCbFn[NUMBER_OF_500HZ_TASKS] =
{
	&RTE_Task500Hz_DT0,
	&RTE_Task500Hz_DT1
};

static const  PVoidCallBackFuncType ACbFn1ms[ESM_NUM_OF_STATES] =
{
  &VoidFn1msInit,
  &VoidFn1msStartup,
  &VoidFn1msFullyOp,
  &VoidFn1msWakeup,
  &VoidFn1msFailSafe,
  &VoidFn1msEolUnlock   
};


/**
*********************************************************************************************************
*  \fn	void RTE_VoidAppTask2KHz(void) 
*
*  \breif 
*	This is a periodical task with 500us period. We can trigger all of functions that require to 500us 
*	tick via call these functions into the this task.
*
*	\warning
*	The execution time of "this task" + "RTE_VoidAppTask1KHz_DT0" shall not greater than 500us.
*	ET(RTE_VoidAppTask2KHz) + ET(RTE_VoidAppTask1KHz_DT0)	< 500us
*
*	\warning
*	The execution time of "this task" + "RTE_VoidAppTask1KHz_DT1" shall not greater than 500us.
*	ET(RTE_VoidAppTask2KHz) + ET(RTE_VoidAppTask1KHz_DT1)	< 500us
*
*********************************************************************************************************
*/
void RTE_Task1KHz(void) 
{
  EnuECUState_t enuEcuState=ESM__FULLY_OPERATIONAL__STATE;
  
  /* The ECU operating state interval is 1ms */
 // enuEcuState = ESM_EnuGetECUState();
   
  /* 
   * ECU state range shall be checked 
   */
  if(enuEcuState < ESM_NUM_OF_STATES)
  {
    /* Monitoring the execution of 1ms tasks */

    /* Calling the relative 1ms function according to the current ECU operating state */
    (*ACbFn1ms[enuEcuState])();
	 
    /* The ECU operating state interval is 1ms */
    //enuEcuState = ESM_EnuECUStateManager();

    
    /* Monitoring the execution of 1ms tasks */

    /* 
	  * ECU state range shall be checked 
	  */
    if(enuEcuState < ESM_NUM_OF_STATES)
    {
      

      /* 
		 * According to the current state of 2ms task(or current delay time) calls the relative 
		 * call-back function 
	    */
      
        (*ATask500Hz_DTxxCbFn[AP2_EnuReadyTask500HZ])(enuEcuState);
      
       
       /* Monitoring the execution of 2ms tasks */
       
      /* Update the AP2_EnuReadyTask500HZ variable to next state 
         according to the EnuTask500HZStates_t */
        VoidUpdateDelayTimesStatus();
    }

  }
  else
  {
    /* Development error shall be set */
  }  
}

/*
*********************************************************************************************************
*
*                                     SOURCES OF PRIVATE FUNCTIONS
*
*********************************************************************************************************
*/
/**
*********************************************************************************************************
*  \fn	void VoidFn1msInit(void) 
*
*  \breif 
*  This function calls all functions with 1ms interval whenever ECU state is init.  
*
*********************************************************************************************************
*/
static void VoidFn1msInit(void)
{

}
/**
*********************************************************************************************************
*  \fn	void VoidFn1msStartup(void) 
*
*  \breif 
*  This function calls all functions with 1ms interval whenever ECU state is statrt-up.  
*
*********************************************************************************************************
*/
static void VoidFn1msStartup(void)
{       
       

}

/**
*********************************************************************************************************
*  \fn	void VoidFn1msFullyOp(void) 
*
*  \breif 
*  This function calls all functions with 1ms interval whenever ECU state is FULLY-OPERATIONAL.  
*
*********************************************************************************************************
*/
static void VoidFn1msFullyOp(void)
{
	   //WCET_Start();       /* Start WCET Measurement */
	MBM_Handler();         /* Modbus-Manager 1*/
	 //  WCET_GetChannel(0); /* Get WCET Value for CH 0 */
	MBS_Handler();         /* Modbus-Manager 2*/
	 //  WCET_GetChannel(1); /* Get WCET Value for CH 1 */
}

/**
*********************************************************************************************************
*  \fn	void VoidFn1msWakeup(void) 
*
*  \breif 
*  This function calls all functions with 1ms interval whenever ECU state is WAKEUP.  
*
*********************************************************************************************************
*/
static void VoidFn1msWakeup(void)
{
  
}

/**
*********************************************************************************************************
*  \fn	void VoidFn1msFailSafe(void) 
*
*  \breif 
*  This function calls all functions with 1ms interval whenever ECU state is FAIL-SAFE.  
*
*********************************************************************************************************
*/
static void VoidFn1msFailSafe(void)
{
}

/**
*********************************************************************************************************
*  \fn	void VoidFn1msEolUnlock(void) 
*
*  \breif 
*  This function calls all functions with 500us interval whenever ECU state is EOL-UNLOCK.  
*
*********************************************************************************************************
*/
static void VoidFn1msEolUnlock(void)
{
	
}
/**
*********************************************************************************************************
*  \fn	void VoidUpdateDelayTimesStatus(void)
*
*  \breif
*  This function obtains the next 1KHz_function(DT0 or DT1) that shall be called by AppTask2KHz.
*
*********************************************************************************************************
*/
static void VoidUpdateDelayTimesStatus(void)
{
	if (TASK_500HZ_DT00 == AP2_EnuReadyTask500HZ)
	{
		AP2_EnuReadyTask500HZ = TASK_500HZ_DT01;
	}
	else
	{
		AP2_EnuReadyTask500HZ = TASK_500HZ_DT00;
	}
}
