/*******************
 ***************************************** C SOURCE FILE ******************************************
 *******************/
/**
 *  @file       WebInsatanceReport.h
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

#ifndef WEBINSTANCEREPORT_H_
#define WEBINSTANCEREPORT_H_

#include "Platform.h"

/*!
 **************************************************************************************************
 *
 *  @fn         WebInsReportUpdate (void)
 *
 *  @par        Contains all Control Algorithms.
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
void WebInsReportUpdate (u8 *u8Name,f32 f32Value);
/*!
 **************************************************************************************************
 *
 *  @fn         WebInsReportUpdate (void)
 *
 *  @par        Contains all Control Algorithms.
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
u8 WebInsReportRead (u8 *u8Str);
/*!
 **************************************************************************************************
 *
 *  @fn         WebInsReportNextEvent (void)
 *
 *  @par        Contains all Control Algorithms.
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
void WebInsReportNextEvent (void);
/*!
 **************************************************************************************************
 *
 *  @fn         WebInsReportStatus (void)
 *
 *  @par        Contains all Control Algorithms.
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
u8 WebInsReportStatus (void);

#endif /* WEBINSTANCEREPORT_H_ */
