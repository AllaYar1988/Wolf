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
#include "myrtc.h"
#include "sysvar.h"
#include "ds1307_for_stm32_hal.h"
#include <stdbool.h>
#include "dbg.h"
   
 DateTime urtc,mdt,dt,urtcd;
char _timeTag=0;
RTC_TimeTypeDef sTime = { 0 };
RTC_DateTypeDef sDate = { 0 };
char dateStr[100];

void _rtcFunctionRead(int read) {
	//DateTime dt;
  char _strdummy[100];
	int tr;
	if (read == 0) {
		dt.day = DS1307_GetDate();
		dt.month = DS1307_GetMonth();
		dt.year = DS1307_GetYear();
		//dow = DS1307_GetDayOfWeek();
		dt.hour = DS1307_GetHour();
		dt.minute = DS1307_GetMinute();
		dt.second = DS1307_GetSecond();
		int fr = 0;					//check_rtc_freeze(&dt);
		if (false == isTimeValid(dt, &tr) || 1 == fr) {
			RTC_GetTime(&hrtc, &dt);
			// TransmitModbus(dateStr);
		}

		if (isTimeValid(dt, &tr)) {					//_rtcOK=1;
													//wrongTimeCount=0;
			_timeTag = 1;
			urtc.day = dt.day;
			urtc.month = dt.month;
			urtc.year = dt.year;
			urtc.hour = dt.hour;
			urtc.minute = dt.minute;
			urtc.second = dt.second;
			//
		} else {			//_rtcOK=0;

			if (isTimeValid(mdt, &tr)) {

				urtc.day = mdt.day;
				urtc.month = mdt.month;
				urtc.year = mdt.year;
				urtc.hour = mdt.hour;
				urtc.minute = mdt.minute;
				urtc.second = mdt.second;
				sprintf(_strdummy, ">> Date & Time Updated by SIM!\r");
				TransmitDebug(_strdummy);
				DS1307_SetTimeZone(+8, 00);
				__uSdelay(200)
				DS1307_SetDate(urtc.day);
				__uSdelay(200)
				DS1307_SetMonth(urtc.month);
				__uSdelay(200)
				DS1307_SetYear(urtc.year);
				__uSdelay(200)
				DS1307_SetDayOfWeek(5);
				__uSdelay(200)
				DS1307_SetHour(urtc.hour);
				__uSdelay(200)
				DS1307_SetMinute(urtc.minute);
				__uSdelay(200)
				DS1307_SetSecond(urtc.second);

				RTC_SetTime(&hrtc, &urtc);
			} else if (isTimeValid(urtc, &tr) == true) {
				DS1307_SetTimeZone(+8, 00);
				DS1307_SetDate(urtc.day);
				DS1307_SetMonth(urtc.month);
				DS1307_SetYear(urtc.year);
				DS1307_SetDayOfWeek(5);
				DS1307_SetHour(urtc.hour);
				DS1307_SetMinute(urtc.minute);
				DS1307_SetSecond(urtc.second);

				RTC_SetTime(&hrtc, &urtc);
			} else {
				//_sendtimeRequest=1;
				sprintf(_strdummy, ">> time request from server!\r");
			}

		}
	} else if (read == 1) {
		sprintf(_strdummy, ">> Date & Time Updated by Server!\r");

		//_sendtimeRequest=0;
	}

}


bool isTimeValid( DateTime dt1,int *timereq) {
	if((dt1.year < YEAR_MIN) || (dt1.year> YEAR_MAX) || (dt1.month < MONTH_MIN) || (dt1.month > MONTH_MAX) ||
			(dt1.day < DAY_MIN) || (dt1.day > DAY_MAX) || (dt1.hour > HOUR_MAX) ||
			(dt1.hour > MINUTE_MAX) || (dt1.second > SECOND_MAX))
	{
          *timereq=1;
		return false;
		
	}
	return true;
}

void RTC_SetTime(RTC_HandleTypeDef *hrtc, DateTime *dt) {
	RTC_TimeTypeDef sTime = { 0 };
	RTC_DateTypeDef sDate = { 0 };

	// Set the time: Hours, Minutes, Seconds
	sTime.Hours = dt->hour;      // Set hours (0-23 for 24-hour format)
	sTime.Minutes = dt->minute;    // Set minutes
	sTime.Seconds = dt->second;    // Set seconds
	sTime.TimeFormat = RTC_HOURFORMAT_24; // Can be used for 12-hour format if needed
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;

	// Set the time in RTC
	if (HAL_RTC_SetTime(hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
		// Error handling
		Error_Handler();
	}

	// Set the date: Weekday, Month, Date, Year
	sDate.WeekDay = RTC_WEEKDAY_MONDAY;
	sDate.Month = dt->month;
	sDate.Date = dt->day;        // Set day of the month
	sDate.Year = dt->year - 2000;      // Set year (last two digits, e.g., 2024)

	// Set the date in RTC
	if (HAL_RTC_SetDate(hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
		// Error handling
		Error_Handler();
	}
}


void RTC_GetTime(RTC_HandleTypeDef *hrtc, DateTime *dt) {

	// Get the current time
	HAL_RTC_GetTime(hrtc, &sTime, RTC_FORMAT_BIN);

	// Get the current date (this should be called after HAL_RTC_GetTime)
	HAL_RTC_GetDate(hrtc, &sDate, RTC_FORMAT_BIN);
	dt->hour = sTime.Hours;
	dt->minute = sTime.Minutes;
	dt->second = sTime.Seconds;
	dt->day = sDate.Date;
	dt->month = sDate.Month;
	dt->year = sDate.Year + 2000;
	// Print the time and date (you can adapt this part for your application)
	// sprintf(dateStr,"Time: %02d:%02d:%02d\n", sTime.Hours, sTime.Minutes, sTime.Seconds);
	// sprintf(dateStr,"%sDate: %02d-%02d-20%02d\n", dateStr,sDate.Date, sDate.Month, sDate.Year);
}


 int rtcSetByServer(DateTime sDt)
 {int timeOk=0;

	 int timereq=0;
	 if(isTimeValid(sDt, &timereq)){
		 urtc.year = sDt.year;
		 urtc.month = sDt.month;
		 urtc.day = sDt.day;
		 urtc.hour = sDt.hour;
		 urtc.minute = sDt.minute;
		 urtc.second = sDt.second;
		 DS1307_SetTimeZone(+8,00);
		 __uSdelay(200)
		 DS1307_SetDate(urtc.day);
		 __uSdelay(200)
		 DS1307_SetMonth(urtc.month);
		 __uSdelay(200)
		 DS1307_SetYear(urtc.year);
		 __uSdelay(200)
		 DS1307_SetDayOfWeek(5);
		 __uSdelay(200)
		 DS1307_SetHour(urtc.hour);
		 __uSdelay(200)
		 DS1307_SetMinute(urtc.minute);
		 __uSdelay(200)
		 DS1307_SetSecond(urtc.second);

		 RTC_SetTime(&hrtc,&urtc);
                 timeOk=1;
	 }
         return timeOk;
 }


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
u8 clkCommand(char *str) {

	if (strstr((char*) str, "set=")) {
		int dt_year, dt_month, dt_day, dt_hour, dt_minute, dt_second;
		int fb = sscanf(str, "clk set=%d/%d/%d,%d:%d:%d", &dt_year, &dt_month,
				&dt_day, &dt_hour, &dt_minute, &dt_second);
		if (fb == 6) {
			urtc.year = dt_year;
			urtc.month = dt_month;
			urtc.day = dt_day;
			urtc.hour = dt_hour;
			urtc.minute = dt_minute;
			urtc.second = dt_second;
			DS1307_SetDate(urtc.day);
			DS1307_SetMonth(urtc.month);
			DS1307_SetYear(urtc.year);
			//DS1307_SetDayOfWeek(1);
			DS1307_SetHour(urtc.hour);
			DS1307_SetMinute(urtc.minute);
			DS1307_SetSecond(urtc.second);

			RTC_SetTime(&hrtc, &urtc);

			sprintf(str, "Date and Time updated by user command:\r");

		}
	} else if (strstr((char*) str, "get")) {
		sprintf(str, " Date:\n %04d/%02d/%02d \n Time: \n %02d:%02d:%02d \n",
				urtc.year, urtc.month, urtc.day, urtc.hour, urtc.minute,
				urtc.second);
	}
	return 0;
}

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
u8 clkCommandHelp(void) {
static u8 state = 0;
u8 returnValue=1;

switch(state)
{
case 0:
	TransmitCMDResponse("     clk get                -> (Returns RTC Date and Time) \r");
	state=1;
	break;
case 1:
	TransmitCMDResponse("     clk set=Date,Time      -> (To set Date/Time : clk=2024/07/30,21:45:00)\r");
	state=2;
	break;
case 2:
	TransmitCMDResponse("     chcom off              -> (To turn charger communication on/off \r");
	state=0;
	returnValue=0;
	break;
}
return returnValue;
}
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
int myrtc_UpdateTime(uint8_t *str, DateTime *mdt) {
	int flag = 0;
	DateTime udt;
	static u8 stcU8timeSet=0;
//char *start=strstr(str,"CLK");
	parseDateTimeFromJson(str);

	if (strstr((char *)str, "+CCLK: \"")) {
    char *sstr=strstr((char *)str, "+CCLK: \"");
		char fb = sscanf(sstr, "+CCLK: \"%2d/%2d/%2d,%2d:%2d:%2d", &udt.year, &udt.month,
				&udt.day, &udt.hour, &udt.minute, &udt.second);
		udt.year = udt.year + 2000;
		if (fb == 6) {
			mdt->year = udt.year;
			mdt->month = udt.month;
			mdt->day = udt.day;
			mdt->hour = udt.hour;
			mdt->minute = udt.minute;
			mdt->second = udt.second;
			if(stcU8timeSet==0)
			{
				stcU8timeSet=1;
			DS1307_SetDate(udt.day);
			DS1307_SetMonth(udt.month);
			DS1307_SetYear(udt.year);
			//DS1307_SetDayOfWeek(1);
			DS1307_SetHour(udt.hour);
			DS1307_SetMinute(udt.minute);
			DS1307_SetSecond(udt.second);

			RTC_SetTime(&hrtc, &udt);
			TransmitDebug("\r Clk Set by Modem \r");
			}
			flag = 1;

		}
	}
	return flag;
}

int parseDateTimeFromJson(char *u8Json)
{
	char u8DateStr[11] = {0}; // "YYYY-MM-DD"
	char u8TimeStr[9]  = {0}; // "HH:MM:SS"
	DateTime stuDt;

	if (NULL!=strstr(u8Json,"date") && NULL!=strstr(u8Json,"time") )
	{
		char *date_ptr = strstr(u8Json, "\"date\":\"");
		char *time_ptr = strstr(u8Json, "\"time\":\"");

		if (date_ptr && time_ptr) {
			sscanf(date_ptr, "\"date\":\"%10[^\"]", u8DateStr);
			sscanf(time_ptr, "\"time\":\"%8[^\"]", u8TimeStr);

			// Parse into integers
			sscanf(u8DateStr, "%4d-%2d-%2d", &stuDt.year, &stuDt.month, &stuDt.day);
			sscanf(u8TimeStr, "%2d:%2d:%2d", &stuDt.hour, &stuDt.minute, &stuDt.second);
			stuDt.msec = 0;
			rtcSetByServer(stuDt);
		}
	}
	return 0;
}

