
/*******************
 ***************************************** C SOURCE FILE ******************************************
 *******************/
/**
 *  @file       RTE_MNT_WEB.c
 *
 *  General Info
 *  ------------
 *  ____
 *  @par        File info
 *  @li @b      Version         : 1.0.4
 *  @li @b      Date            : 2024-05-04
 *
 *  @par        Project info
 *  @li @b      Project         : xxxxx
 *  @li @b      Processor       : STM32f4xx
 *  @li @b      Tool  @b Chain  :
 *  @li @b      Clock @b Freq   : 168 MHz
 *
 *  @par        This file create for System Memory and Web Communication State Machine
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
 *  @li @b      Date            : 2024-05-04
 *  @li @b      Author          : A.Moazami
 *  @li @b      Approved @b by  :
 *  @li @b      Description
 *
 *              Initial Version
 *
 **************************************************************************************************
 */
#include "RTE_MNT_WEB.h"
#include "main.h"
#include "httpFrame.h"
#include "MAC.h"
#include "dbg.h"
#include "iflash.h"
#include "myrtc.h"
#include "fatfs.h"
#include "MEM.h"
#include "crc.h"
#include "MdmSrv.h"
#include "mntdata.h"
#include "WebInstanceReport.h"
#include "inv_fault_recorder.h"



#define MODEM_TICK               500     /*ms*/
#define MONITORING_TICK          100     /*ms*/
#define LOG_TICK                 60      /*Sec*/


WEB_MNG_STU_Type StuWebMng;
LOG_MNG_STU_Type StuLogMng;
static u16  StcU16MdmReady=0;

void RTE_MNT_WEB_MNG(void);
void RTE_MNT_LOG_MNG(void);
static void RTE_MEM_HANDLE(void);
static void RTE_RstReportRefToWeb(void);
static u8 RefDataSend(void);
/*!
 **************************************************************************************************
 *
 *  @fn         void RTE_MNT_MNG(void)
 *
 *  @par        This function is for RTE Monitoring State Manager
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
void RTE_MNT_MNG(void)
{
	static RTE_MNT_Enu mntState = RTE_MNT_ENTRY;

	switch (mntState) {
	case RTE_MNT_ENTRY:

		StuWebMng.tick = HAL_GetTick();
		StuWebMng.state = WEB_MNG_ENTRY;

		StuLogMng.tick = HAL_GetTick();
		StuLogMng.logtick = 0; //HAL_GetTick();
		StuLogMng.state = LOG_MNG_ENTRY;

		mntState = RTE_MNT_DO;
		break;
	case RTE_MNT_DO:

		RTE_MEM_HANDLE();
		RTE_MNT_LOG_MNG();
		RTE_MNT_WEB_MNG();
		break;
	}

}
/*!
 **************************************************************************************************
 *
 *  @fn         void RTE_MNT_WEB_MNG(void)
 *
 *  @par        This function is for Handling Web Communication
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
void RTE_MNT_WEB_MNG(void)
{
	mdmResProcess();
	if ((HAL_GetTick() - StuWebMng.tick) >= MODEM_TICK) {
		StuWebMng.tick = HAL_GetTick();
		switch (StuWebMng.state) {
		case WEB_MNG_ENTRY:
			StuWebMng.state = WEB_MNG_IDLE;
			break;
		case WEB_MNG_IDLE:
			if (1 == mdmGprs.busy) {
				StuWebMng.state = WEB_MNG_SENDING;
			}
			break;
		case WEB_MNG_SENDING:
			ModuleHandle(&mdmGprs);
			if (0 == mdmGprs.busy) {
				StuWebMng.state = WEB_MNG_IDLE;
				TransmitDebug("Modem is free\r");
			}
			break;
		case WEB_MNG_GET_MODEM_CLK:
			break;
		case WEB_MNG_GET_SERVER_CLK:
			break;
		default:
			break;
		}

	} else {

	}

}
/*!
 **************************************************************************************************
 *
 *  @fn         void RTE_MNT_LOG_MNG(void)
 *
 *  @par        This function to Handle Data Log
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
void RTE_MNT_LOG_MNG(void)
{
	u8 aU8Str[50];
	static u32 stcU32MdmCnt=0;

	static u8  stcU8CfgSendDone=1;


	if ((HAL_GetTick() - StuLogMng.tick) >= MONITORING_TICK) {
		StuLogMng.tick = HAL_GetTick();
		switch (StuLogMng.state) {
		case LOG_MNG_ENTRY:
			StuLogMng.state = LOG_MNG_IDLE;
			break;
		case LOG_MNG_IDLE:
			StuLogMng.pTime = (uint16_t) ((HAL_GetTick() - StuLogMng.logtick) / 1000);
			if (StuLogMng.pTime++ >= LOG_TICK) {
				StuLogMng.logtick = HAL_GetTick();
				StuLogMng.cDataInMem = 1;
				TransmitDebug("New Log to Send\r");
				_rtcFunctionRead(0);

			}
			if(stcU32MdmCnt++>300)
			{
				StcU16MdmReady=1;
			}

			RTE_RstReportRefToWeb();

			if(1==getSendCfg() && 1==StcU16MdmReady)
			{
				stcU8CfgSendDone=0;
				setSendCfg(0);
			}
			if (0 == mdmGprs.busy && StcU16MdmReady==1)
			{

				if(1==inv_fault_recorder_status())
				{
					inv_fault_recorder_web_report(mdmGprs.sData,mdmGprs.api);
					mdmGprs.busy = 1;
					mdmGprs.response=0;
				}
				else if(0==stcU8CfgSendDone)
				{
					stcU8CfgSendDone=RefDataSend();
				}
				else if (_readreq != 0)
				{
					mdmGprs.busy = 1;
				}
				else if (0 != StuLogMng.cDataInMem) {
					memset(mdmGprs.sData, 0, BUFFER_SIZE);
					server_return_url(aU8Str);
					sprintf(mdmGprs.api,"%s/api/send-chanel",aU8Str);
					mdmGprs.response=1;
					_rtcFunctionRead(0);
					startJsonFrame((char*) mdmGprs.sData, urtc);
					if (2 != addDataToJsonFrame((char*) mdmGprs.sData)) {
						sprintf((char *)aU8Str, ">>New packet to send\r");
						TransmitDebug((char *)aU8Str);
						mdmGprs.busy = 1;
					} else {
						mdmGprs.busy = 0;
						StuLogMng.cDataInMem = 0;
						sprintf((char *)aU8Str, ">>Waiting for new Data\r");
						TransmitDebug((char *)aU8Str);
					}
				}

			}
			break;
		case LOG_MNG_SAVE:
			break;
		default:
			break;
		}
		StuLogMng.tick = HAL_GetTick();
	} else {

	}

}
/*!
 **************************************************************************************************
 *
 *  @fn         static void RTE_MEM_HANDLE(void)
 *
 *  @par        This function to Handle Memory
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
static void RTE_MEM_HANDLE(void)
{
	static MEM_STATES state=MEM_INIT;
	//static u16 stcU16ReadCfgFlg=1;
	static u16 stcU16Idx=0;
	static u16 stcU16Cycle=0;
	static u16 stcU16cnt=0;
	static u8  stcU8WriteFlg=0;
	RefDataType *rData=getRefData();
	u8 u8MemStatus=0;

	if(stcU16Cycle++>MEMORY_TICK)
	{
		stcU16Cycle=0;
		u8 u8status=MEM_SdStatusCheck();

		if(1==u8status)
		{
			state=MEM_IDLE;
		}
		else if(2==u8status)
		{
			state=MEM_WAIT;
		}
		else
		{

		}

		switch(state)
		{
		case MEM_IDLE:

			break;
		case MEM_WAIT:
			if(stcU16cnt++>100)
			{
				stcU16cnt=0;
				while(1);
				//state=MEM_DISK_INIT;
			}
			break;
		case MEM_DISK_INIT:
			SD_init(0);
			state=MEM_INIT;
			break;
		case MEM_INIT:
			u8MemStatus=memHnadler(0,MEM_INIT);
			if(1==u8MemStatus)
			{
				state=MEM_READ_CONFIG;
			}
			else if(2==u8MemStatus)
			{
				state=MEM_DISK_INIT;
			}

			break;
		case MEM_SAVE_CONFIG:

			if((rData[stcU16Idx].flag&0x02)==0x02)
			{
				u8MemStatus=memHnadler(stcU16Idx,MEM_SAVE_CONFIG);
				if(3==u8MemStatus)
				{
					stcU8WriteFlg=1;
				}
				else if(2==u8MemStatus)
				{
					state=MEM_INIT;
				}
				else if(4==u8MemStatus && stcU8WriteFlg==1)
				{
					rData[stcU16Idx].flag&=0xFD;
					stcU8WriteFlg=0;

				}
			}
			else
			{
				if(++stcU16Idx>=(REF_ARRAY_SIZE-1))stcU16Idx=0;
			}
			break;
		case MEM_READ_CONFIG:
			u8MemStatus=memHnadler(stcU16Idx,MEM_READ_CONFIG);
			if(1==u8MemStatus)
			{
				if(++stcU16Idx>=(REF_ARRAY_SIZE-1))
				{
					stcU16Idx=0;
					state=MEM_SAVE_CONFIG;
				//stcU16ReadCfgFlg=0;
				}
			}
			else if(2==u8MemStatus)
			{

				state=MEM_INIT;

			}
			break;
		}

	}

}

/*!
 **************************************************************************************************
 *
 *  @fn         static void RTE_RstReportRefToWeb(void)
 *
 *  @par        This function to reset send ref to web flag
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
static void RTE_RstReportRefToWeb(void)
{
	u8 u8WebRefIdSts=MdmSrv_GetWebRefIdSts();
	if(1==u8WebRefIdSts)
	{

		RefDataType *rData=getRefData();
		f32 f32RefId=getRefIDValue();
		u16 u16RefCount=MdmSrv_GetWebRefId();

		MdmSrv_SetWebRefIdSts(0);

		if(f32RefId<=u16RefCount)
		{
			for (int i = 0; i < REF_ARRAY_SIZE; i++)
			{
				rData[i].flag&=~(REF_REPORT_TO_WEB);
			}
		}
		else
		{
			setSendCfg(1);
		}
	}
}

/*!
 **************************************************************************************************
 *
 *  @fn         static void RefDataSend(void)
 *
 *  @par        This function to reset send ref to web flag
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
static u8 RefDataSend(void)
{
	static u32 stcU16CfgIdx=0U;
	u8 u8ReturnValue=0U;
	u8 str[200]={0};
	u8 u8BodyStr[512]={0};
	u8 u8FirstEntry=0;
	u8 u8Index=0;
	RefDataType *rData=getRefData();

	memset(mdmGprs.sData, 0, BUFFER_SIZE);
	_rtcFunctionRead(0);
    startJsonFrame((char*) str, urtc);

for(;;)
{
	if((rData[stcU16CfgIdx].flag & REF_REPORT_TO_WEB) ==REF_REPORT_TO_WEB )
	{
        if (1==u8FirstEntry)
        {
            strcat(u8BodyStr, ",\r"); // Add comma BEFORE item if it's not the first

        }
        u8FirstEntry=1;
        sprintf(u8BodyStr,"%s    \"%s\": \"%s*%0.1f*%s\"",
        		u8BodyStr,
					rData[stcU16CfgIdx].ref,
					rData[stcU16CfgIdx].name,
					rData[stcU16CfgIdx].value,
					rData[stcU16CfgIdx].unit);
			mdmGprs.busy = 1;
			rData[stcU16CfgIdx].flag&=~(REF_REPORT_TO_WEB );
			if(++u8Index>6)
			{
				break;
			}
	}
	if(stcU16CfgIdx<getRefIDIndex())
	{
		stcU16CfgIdx++;
		u8ReturnValue=0U;
	}
	else
	{
		u8ReturnValue=1U;
		stcU16CfgIdx=0;
		break;
	}
}
	sprintf(mdmGprs.sData,"%s%s\r}\r}",str, u8BodyStr);

return u8ReturnValue;
}
