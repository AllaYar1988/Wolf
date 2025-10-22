                                      /*******************
 ***************************************** C SOURCE FILE ******************************************
                                        *******************/
/**
 *  @file       myrtc.C/.H
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
 *  @par        This file create for use Media Access Control (MAC) for UART
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
 *  @li @b      Date            : 2025-05-13
 *  @li @b      Author          : A.Moazami
 *  @li @b      Approved @b by  :
 *  @li @b      Description
 *
 *              Initial Version
 *
 **************************************************************************************************
 */

#ifndef _MYRTC_H
#define _MYRTC_H

#include "Platform.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "rtc.h"

#define __uSdelay(x)          for(int i=0;i<x;i++){};
#define YEAR_MIN 2023
#define YEAR_MAX 2050
#define MONTH_MIN 1
#define MONTH_MAX 12
#define DAY_MIN 1
#define DAY_MAX 31
#define HOUR_MIN 0
#define HOUR_MAX 24
#define MINUTE_MIN 0
#define MINUTE_MAX 60
#define SECOND_MIN 0
#define SECOND_MAX 60

   typedef struct {
	int hour;
	int minute;
	int second;
	int msec;
	int day;
	int month;
	int year;
	int index;
} DateTime;
   
   extern DateTime urtc,mdt,dt,urtcd;
   extern char _timeTag;

bool isTimeValid( DateTime dt,int *timereq) ;
   void _rtcFunctionRead(int read);
   void RTC_SetTime(RTC_HandleTypeDef *hrtc, DateTime *dt) ;
void RTC_GetTime(RTC_HandleTypeDef *hrtc, DateTime *dt) ;
 int rtcSetByServer(DateTime sDt);

 /*!
  **************************************************************************************************
  *
  *  @fn         int myrtc_UpdateTime(uint8_t *str, DateTime *mdt)
  *
  *  @par        This function to Update  Time.
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
 int myrtc_UpdateTime(uint8_t *str, DateTime *mdt);
 /*!
  **************************************************************************************************
  *
  *  @fn         u8 clkCommand(char *str)
  *
  *  @par        This function is for Set Set/Get/Save CLK.
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
 u8 clkCommand(char *str) ;

 /*!
  **************************************************************************************************
  *
  *  @fn         u8 clkCommandHelp(void)
  *
  *  @par        This function is for Set Set/Get/Save CLK.
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
 u8 clkCommandHelp(void);

 int parseDateTimeFromJson(char *u8Json);

#endif // _MYRTC_H
