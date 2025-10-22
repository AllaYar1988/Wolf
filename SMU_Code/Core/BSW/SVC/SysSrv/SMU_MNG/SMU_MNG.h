
                                        /*******************
***************************************** C SOURCE FILE ******************************************
                                        *******************/
/**
*  @file       SMU_MNG_StateManager.c/.h
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
*  @copyright  (C) 2017-2019 CROUSE PJS Inc. All rights reserved.
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

#ifndef _SMU_MNG_H_
#define _SMU_MNG_H_




/** \typedef (EnuECUState_t) It specifies the available operating states */
typedef enum
{
 	ESM__INIT__STATE                 = 0U,
	ESM__STARTUP__STATE              = 1U,
	ESM__FULLY_OPERATIONAL__STATE    = 2U,
	ESM__WAKEUP__STATE               = 3U,
	ESM__FAIL_SAFE__STATE            = 4U,
	ESM__EOL_UNLOCK__STATE           = 5U,

   ESM_NUM_OF_STATES                = 6U

} EnuECUState_t;

/** \typedef (EnuECUStateTransition_t) It specifies the available transition between variouse states */
typedef enum
{
  /* Init */
  INIT__TO__STARTUP = 1U,

  /* Start-up */
  STARTUP__TO__FULLY_OP = 2u,
  STARTUP__TO__FAIL_SAFE = 3u,
  STARTUP__TO__EOL_UNLOCK = 4u,

  /* Fully operational */
  FULLY_OP__TO__FAIL_SAFE = 5u,
  FULLY_OP__TO__WAKEUP = 6u,

  /* Wakeup */
  WAKEUP__TO__FULLY_OP = 7u,

  /* Fail-safe */

  /* UNLOCK */

  MAX_NUM_OF_TRANSITION = 8u

}EnuECUStateTransition_t;



typedef struct
{
  EnuECUState_t enuNewECUState;
  EnuECUState_t enuOldECUState;
  EnuECUStateTransition_t enuECUStateTransition;
}StrECUStateMng_t;




/**
***************************************************************************************************
*  \fn EnuECUState_t ESM_EnuECUStateManager(void)
*
*  \breif
*	Checks the current ECU state and according to the current conditions decides about next state.
*	This function shall be call every 250us
*
*	\return
*	This function returns the current ACU operating state.
*
***************************************************************************************************
*/
EnuECUState_t SMU_MNG_StateManager(void);



#endif /* _SMU_MNG_H_ */
