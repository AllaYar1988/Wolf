
                                        /*******************
***************************************** C SOURCE FILE ******************************************
                                        *******************/
/**
*  @file      RTE_Task500Hz_DT0.c/.h
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
#include "RTE_Task500Hz_DT1.h"
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

u16 U16Pr2msCounter = 0u;

/*
*********************************************************************************************************
*
*                                     PROTOTYPES OF PRIVATE FUNCTIONS
*
*********************************************************************************************************
*/
static void VoidFn2msInitDT1(void);
static void VoidFn2msStartupDT1(void);
static void VoidFn2msFullyOpDT1(void);
static void VoidFn2msWakeupDT1(void);
static void VoidFn2msFailSafeDT1(void);
static void VoidFn2msEolUnlockDT1(void);

/*
*********************************************************************************************************
*
*                                           PRIVATE VARIABLES
*
*********************************************************************************************************
*/
static const PVoidCallBackFuncType ACbFn2ms_DT1[ESM_NUM_OF_STATES] =
{
  &VoidFn2msInitDT1,
  &VoidFn2msStartupDT1,
  &VoidFn2msFullyOpDT1,
  &VoidFn2msWakeupDT1,
  &VoidFn2msFailSafeDT1,
  &VoidFn2msEolUnlockDT1
};

static u16 U16Mod_005   = 0u;
static u16 U16Mod_010   = 0u;
static u16 U16Mod_025   = 0u;
static u16 U16Mod_050   = 0u;
static u16 U16Mod_100   = 0u;


/*
*********************************************************************************************************
*
*                                     SOURCES OF PRIVATE FUNCTIONS
*
*********************************************************************************************************
*/

/**
*********************************************************************************************************
*  \fn	void VoidFn1msInitDT1(void) 
*
*  \breif 
*  This function calls all functions with 1ms interval whenever ECU state is init.  
*
*********************************************************************************************************
*/
static void VoidFn2msInitDT1(void)
{
}
/**
*********************************************************************************************************
*  \fn	void VoidFn1msStartupDT1(void) 
*
*  \breif 
*  This function calls all functions with 2ms interval whenever ECU state is statrt-up.
*
*********************************************************************************************************
*/
static void VoidFn2msStartupDT1(void)
{
	/*
		 **************************************
		 *	 2ms activities shall be call here
		 **************************************
		 */

		/*
		 **********************************************************************
		 *	 Activities that need to greater period than 2ms shall be call here
		 **********************************************************************
		 */

		/*
		 * Task 10ms Offset 0*2ms
		 */
		if (0u == U16Mod_005)
		{
		}

		/*
		 * Task 10ms Offset 1*2ms
		 */
		else if (1u == U16Mod_005)
		{

		}

		/*
		 * Task 20ms Offset 2*2ms
		 */
		else if (2u == U16Mod_010)
		{

		}

		/*
		 * Task 20ms Offset 3*2ms
		 */
		else if (3u == U16Mod_010)
		{

		}

		/*
		 * Task 50ms Offset 4*2ms
		 */
		else if (4u == U16Mod_025)
		{
		}

		/*
		 * Task 100ms Offset 7*2ms
		 */
		else if (7u == U16Mod_050)
		{
		}

		/*
		 * Task 200ms Offset 8*2ms
		 */
		else if (8u == U16Mod_100)
		{
		}

		/*
		 * Task 100ms Offset 9
		 */
		else if (9u == U16Mod_100)
		{
		}

	   /*
	   * MISRA-C complaiance
	   */
		else
		{
			/* Do Nothing */
		}
}

/**
*********************************************************************************************************
*  \fn	void VoidFn2msFullyOpDT1(void)
*
*  \breif 
*  This function calls all functions with 2ms interval whenever ECU state is Fully Operational.
*
*********************************************************************************************************
*/
static void VoidFn2msFullyOpDT1(void)
{
	/*
		 **************************************
		 *	 2ms activities shall be call here
		 **************************************
		 */

		/*
		 **********************************************************************
		 *	 Activities that need to greater period than 2ms shall be call here
		 **********************************************************************
		 */

		/*
		 * Task 10ms Offset 0*2ms
		 */
		if (0u == U16Mod_005)
		{
		}

		/*
		 * Task 10ms Offset 1*2ms
		 */
		else if (1u == U16Mod_005)
		{

		}

		/*
		 * Task 20ms Offset 2*2ms
		 */
		else if (2u == U16Mod_010)
		{

		}

		/*
		 * Task 20ms Offset 3*2ms
		 */
		else if (3u == U16Mod_010)
		{

		}

		/*
		 * Task 50ms Offset 4*2ms
		 */
		else if (4u == U16Mod_025)
		{
		}

		/*
		 * Task 100ms Offset 7*2ms
		 */
		else if (7u == U16Mod_050)
		{
		}

		/*
		 * Task 200ms Offset 8*2ms
		 */
		else if (8u == U16Mod_100)
		{
		}

		/*
		 * Task 100ms Offset 9
		 */
		else if (9u == U16Mod_100)
		{
		}

	   /*
	   * MISRA-C complaiance
	   */
		else
		{
			/* Do Nothing */
		}
}

/**
*********************************************************************************************************
*  \fn	void VoidFn2msWakeupDT1(void)
*
*  \breif 
*  This function calls all functions with 2ms interval whenever ECU state is WAKEUP.
*
*********************************************************************************************************
*/
static void VoidFn2msWakeupDT1(void)
{
	/*
		 **************************************
		 *	 2ms activities shall be call here
		 **************************************
		 */

		/*
		 **********************************************************************
		 *	 Activities that need to greater period than 2ms shall be call here
		 **********************************************************************
		 */

		/*
		 * Task 10ms Offset 0*2ms
		 */
		if (0u == U16Mod_005)
		{
		}

		/*
		 * Task 10ms Offset 1*2ms
		 */
		else if (1u == U16Mod_005)
		{

		}

		/*
		 * Task 20ms Offset 2*2ms
		 */
		else if (2u == U16Mod_010)
		{

		}

		/*
		 * Task 20ms Offset 3*2ms
		 */
		else if (3u == U16Mod_010)
		{

		}

		/*
		 * Task 50ms Offset 4*2ms
		 */
		else if (4u == U16Mod_025)
		{
		}

		/*
		 * Task 100ms Offset 7*2ms
		 */
		else if (7u == U16Mod_050)
		{
		}

		/*
		 * Task 200ms Offset 8*2ms
		 */
		else if (8u == U16Mod_100)
		{
		}

		/*
		 * Task 100ms Offset 9
		 */
		else if (9u == U16Mod_100)
		{
		}

	   /*
	   * MISRA-C complaiance
	   */
		else
		{
			/* Do Nothing */
		}
}

/**
*********************************************************************************************************
*  \fn	void VoidFn2msFailSafeDT1(void)
*
*  \breif 
*  This function calls all functions with 2ms interval whenever ECU state is FAIL-SAFE.
*
*********************************************************************************************************
*/
static void VoidFn2msFailSafeDT1(void)
{
	/*
	 **************************************
	 *	 2ms activities shall be call here
	 **************************************	
	 */
	
	/*
	 **********************************************************************
	 *	 Activities that need to greater period than 2ms shall be call here
	 **********************************************************************
	 */
	
	/*
	 * Task 10ms Offset 0*2ms
	 */	 
	if (0u == U16Mod_005)
	{
	}
	
	/* 
	 * Task 10ms Offset 1*2ms
	 */
	else if (1u == U16Mod_005)
	{
       
	}
	
	/* 
	 * Task 20ms Offset 2*2ms
	 */
	else if (2u == U16Mod_010)
	{
     
	}
	
	/* 
	 * Task 20ms Offset 3*2ms
	 */
	else if (3u == U16Mod_010)
	{
      
	}
	
	/* 
	 * Task 50ms Offset 4*2ms
	 */
	else if (4u == U16Mod_025)
	{
	}
	
	/* 
	 * Task 100ms Offset 7*2ms
	 */	
	else if (7u == U16Mod_050)
	{
	}
	
	/* 
	 * Task 200ms Offset 8*2ms
	 */
	else if (8u == U16Mod_100)
	{
	}
	
	/* 
	 * Task 100ms Offset 9 
	 */
	else if (9u == U16Mod_100)
	{
	}
   
   /* 
   * MISRA-C complaiance
   */
	else
	{
		/* Do Nothing */
	}		
}

/**
*********************************************************************************************************
*  \fn	void VoidFn2msEolUnlockDT1(void)
*
*  \breif 
*  This function calls all functions with 2ms interval whenever ECU state is EOL-UNLOCK.
*
*********************************************************************************************************
*/
static void VoidFn2msEolUnlockDT1(void)
{
	/*
		 **************************************
		 *	 2ms activities shall be call here
		 **************************************
		 */

		/*
		 **********************************************************************
		 *	 Activities that need to greater period than 2ms shall be call here
		 **********************************************************************
		 */

		/*
		 * Task 10ms Offset 0*2ms
		 */
		if (0u == U16Mod_005)
		{
		}

		/*
		 * Task 10ms Offset 1*2ms
		 */
		else if (1u == U16Mod_005)
		{

		}

		/*
		 * Task 20ms Offset 2*2ms
		 */
		else if (2u == U16Mod_010)
		{

		}

		/*
		 * Task 20ms Offset 3*2ms
		 */
		else if (3u == U16Mod_010)
		{

		}

		/*
		 * Task 50ms Offset 4*2ms
		 */
		else if (4u == U16Mod_025)
		{
		}

		/*
		 * Task 100ms Offset 7*2ms
		 */
		else if (7u == U16Mod_050)
		{
		}

		/*
		 * Task 200ms Offset 8*2ms
		 */
		else if (8u == U16Mod_100)
		{
		}
		
		/*
		 * Task 100ms Offset 9
		 */
		else if (9u == U16Mod_100)
		{
		}

	   /*
	   * MISRA-C complaiance
	   */
		else
		{
			/* Do Nothing */
		}
}

/*
*********************************************************************************************************
*
*                                     SOURCES OF PUBLIC FUNCTIONS
*
*********************************************************************************************************
*/


/**
*********************************************************************************************************
*  \fn	void RTE_VoidAppTask500Hz_DT1(EnuECUState_t enuECUState)
2***********************************************************************************************
*/

void RTE_Task500Hz_DT1(EnuECUState_t enuECUState)
{
  /* Calculating the mode of 1msCounter to  5, 10, 25, 50, 100 */
  U16Mod_005  = U16Pr2msCounter % 5u;
  U16Mod_010  = U16Pr2msCounter % 10u;
  U16Mod_025  = U16Pr2msCounter % 25u;
  U16Mod_050  = U16Pr2msCounter % 50u;
  U16Mod_100  = U16Pr2msCounter % 100u;
  
  /* 
   * Array index range checking 
   */
  if (enuECUState < ESM_NUM_OF_STATES)
  {
    (*ACbFn2ms_DT1[enuECUState])();
  }

  /* Increment the 1ms timer counter by one */
  U16Pr2msCounter++;

  /* 
   * 2ms counter threshold(100) shall be check 
   */
  if(100u == U16Pr2msCounter)
  {
    U16Pr2msCounter = 0u;
  }
  
}

/*
*********************************************************************************************************
*
*                                              END OF FILE
*
*********************************************************************************************************
*/
