/*******************
 ***************************************** C SOURCE FILE ******************************************
 *******************/
/**
 *  @file       WebInsatanceReport.C
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
#include "WebInstanceReport.h"
#include "httpFrame.h"
#include "mntdata.h"
#include "myrtc.h"
/*!
 **************************************************************************************************
 *
 *  @fn         Global Private Var Define
 *
 **************************************************************************************************
 */
#define EVENT_HISTORY_SIZE 255

static u16 StcU16HeadIndex=0;
static u16 StcU16TailIndex=0;

MntDataType StuEventData[EVENT_HISTORY_SIZE];

/*!
 **************************************************************************************************
 *
 *  @fn         Private Function Declaration
 *
 **************************************************************************************************
 */


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
void WebInsReportUpdate (u8 *u8Name,f32 f32Value)
{

	snprintf(StuEventData[StcU16HeadIndex].name,NAME_SIZE,"%s",u8Name);
	snprintf(StuEventData[StcU16HeadIndex].unit,UNIT_SIZE,"N");
	StuEventData[StcU16HeadIndex].value=f32Value;
	if(StcU16HeadIndex++>=EVENT_HISTORY_SIZE)
	{
		StcU16HeadIndex=0;

	}

}
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
u8 WebInsReportRead (u8 *u8Str)
{
	u8 u8StrFrameHead[100];
	u8 u8ReturnValue=0;

	if(StcU16TailIndex!=StcU16HeadIndex)
	{
	_rtcFunctionRead(0);
	startJsonFrame((char*) u8StrFrameHead, urtc);
	sprintf(u8Str,"%s    \"V1\": \"%s*%0.1f*%s\"\r}\r}",
			u8StrFrameHead,
			StuEventData[StcU16TailIndex].name,
			StuEventData[StcU16TailIndex].value,
			StuEventData[StcU16TailIndex].unit);
	u8ReturnValue=0;
	}
	else
	{
		u8ReturnValue=1;
	}

	return u8ReturnValue;
}
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
void WebInsReportNextEvent (void)
{
	if(StcU16TailIndex!=StcU16HeadIndex)
	{
		StcU16TailIndex++;
		if(StcU16TailIndex>EVENT_HISTORY_SIZE)
		{
			StcU16TailIndex=0;

		}
	}

}

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
u8 WebInsReportStatus (void)
{
	u8 u8ReturnValue=0;
	if(StcU16TailIndex!=StcU16HeadIndex)
	{
		u8ReturnValue=0;

	}
	else
	{
		u8ReturnValue=1;
	}
return u8ReturnValue;
}



