
                                        /*******************
***************************************** C SOURCE FILE ******************************************
                                        *******************/
/**
*  @file       WCET.c/.h
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
*              This file is generated in order to Calc WCET Time.
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
*  @li @b      Date            : 5/31/2025
*  @li @b      Author          : Allahyar Moazami
*  @li @b      Approved @b by  :
*  @li @b      Description
*
*              Revision Tag : Enter revision tag related to the current revision.
*              Enter a paragraph that serves as a detail description.
*
**************************************************************************************************
*/
#ifndef _WCET_H
#define _WCET_H

#include "Platform.h"


  extern u32 wcetArray[20];

  /*!
   **************************************************************************************************
   *
   *  @fn         void initDwt(void)
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
  void WCET_InitDwt(void);
  /*!
   **************************************************************************************************
   *
   *  @fn         void WCET_Update(u32 *u32Wcet)
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
  void WCET_Update(u32 *u32Wcet);
  /*!
   **************************************************************************************************
   *
   *  @fn         void setDwt(u32 u32Wcet)
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
  void WCET_SetDwt(u32 u32Wcet);
#endif // WCET_H
