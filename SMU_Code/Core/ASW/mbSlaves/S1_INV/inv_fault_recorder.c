/*
 * inv_fault_recorder.c
 *
 *  Created on: Oct 13, 2025
 *      Author: Allayar Moazami
 */
#include "inv_fault_recorder.h"
#include "httpFrame.h"
#include <stdio.h>
#include <string.h>
#include "server.h"

static StuSnapShotType gStuSnapshot;
static u8 gLocalReport=0;

static void inv_fault_recorder_start_json( u8 *str,DateTime tm);
static u8 iEventBuff[500];
volatile u16 tIdx=0;
/*
 *
 */
void inv_fault_recorder_set_data(u16 index,u16 invData)
{
    u16 hIdx=gStuSnapshot.head;
	if(hIdx<SNAP_SHOT_BUFFER_SIZE)
	{
	gStuSnapshot.node[hIdx].data[index].U=invData;
	}

}

/*
 *
 */
u8 inv_fault_recorder_inc_idx(void)
{
	u8 returnValue=0;
	if(gStuSnapshot.head<SNAP_SHOT_BUFFER_SIZE)
	{
		gStuSnapshot.head++;
		returnValue=1;
	}
	return returnValue;
}
/*
 *
 */
u8 inv_fault_recorder_status(void)
{
	u8 returnValue=0;
	if(0==gLocalReport)
	{
	if(gStuSnapshot.head!=0)
	{
		returnValue=1;
	}
	}
	else
	{
		if(gStuSnapshot.head>=98)
		{
			returnValue=2;
		}
	}
	return returnValue;
}


/*
 *
 * */
void inv_fault_recorder_web_report(u8 *str,u8 *api)
{

	int16_t temp=0;
	u8 tempStr[40];

	tIdx=gStuSnapshot.tail;
	server_return_url(tempStr);
	sprintf(api,"%s/api/send-event",tempStr);
	if(gStuSnapshot.head!=gStuSnapshot.tail)
	{
	_rtcFunctionRead(0);
	inv_fault_recorder_start_json((char*) str, urtc);
	u16 tmp=gStuSnapshot.node[tIdx].data[ENU_SNAP_FCODE].U;
	sprintf((char *)tempStr,"\"V1\": \"%s*%u*%s\",\r",
			"Fcode",
			tmp,
			"N");
	strcat((char *)str,(const char *)tempStr);
	tmp=gStuSnapshot.node[tIdx].data[ENU_SNAP_STATE].U;
	sprintf((char *)tempStr,"\"V2\": \"%s*%u*%s\",\r",
			"State",
			tmp,
			"N");
	strcat((char *)str,(const char *)tempStr);
	temp=gStuSnapshot.node[tIdx].data[ENU_SNAP_CU_INV1].S;
	sprintf((char *)tempStr,"\"V3\": \"%s*%d*%s\",\r",
			"Iinv1",
			temp,
			"A");
	strcat((char *)str,(const char *)tempStr);
	temp=gStuSnapshot.node[tIdx].data[ENU_SNAP_CU_INV2].S;
	sprintf((char *)tempStr,"\"V4\": \"%s*%d*%s\",\r",
			"Iinv2",
			temp,
			"A");
	strcat((char *)str,(const char *)tempStr);
	temp=gStuSnapshot.node[tIdx].data[ENU_SNAP_CU_G1].S;
	sprintf((char *)tempStr,"\"V5\": \"%s*%d*%s\",\r",
			"Ig1",
			temp,
			"A");
	strcat((char *)str,(const char *)tempStr);
	temp=gStuSnapshot.node[tIdx].data[ENU_SNAP_CU_G2].S;
	sprintf((char *)tempStr,"\"V6\": \"%s*%d*%s\",\r",
			"Ig2",
			temp,
			"A");
	strcat((char *)str,(const char *)tempStr);
	temp=gStuSnapshot.node[tIdx].data[ENU_SNAP_VC1].S;
	sprintf((char *)tempStr,"\"V7\": \"%s*%d*%s\",\r",
			"Vc1",
			temp,
			"V");
	strcat((char *)str,(const char *)tempStr);
	temp=gStuSnapshot.node[tIdx].data[ENU_SNAP_VC2].S;
	sprintf(tempStr,"\"V8\": \"%s*%d*%s\",\r",
			"Vc2",
			temp,
			"V");
	strcat((char *)str,(const char *)tempStr);
	temp=gStuSnapshot.node[tIdx].data[ENU_SNAP_VDC_FIL].S;
	sprintf((char *)tempStr,"\"V9\": \"%s*%d*%s\",\r",
			"VDC_FIL",
			temp,
			"V");
	strcat((char *)str,(const char *)tempStr);
	 tmp=gStuSnapshot.node[tIdx].data[ENU_SNAP_STIME].U;
	sprintf(tempStr,"\"V10\": \"%s*%u*%s\",\r",
			"T_Sec",
			tmp,
			"S");
	strcat((char *)str,(const char *)tempStr);
	 tmp=gStuSnapshot.node[tIdx].data[ENU_SNAP_UTIME].U;
	sprintf((char *)tempStr,"\"V11\": \"%s*%u*%s\"\r}\r}",
			"T_uSec",
			tmp,
			"uS");
	strcat((char *)str,(const char *)tempStr);

	gStuSnapshot.tail++;
	}
	else
	{
		gStuSnapshot.head=0;
		gStuSnapshot.tail=0;
		gStuSnapshot.id+=1;
	}
}

void inv_fault_recorder_report()
{

	int16_t temp=0;
	u8 tempStr[40];

	tIdx=gStuSnapshot.tail;

	if(gStuSnapshot.head!=gStuSnapshot.tail)
	{
	_rtcFunctionRead(0);
	inv_fault_recorder_start_json((char*) iEventBuff, urtc);
	u16 tmp=gStuSnapshot.node[tIdx].data[ENU_SNAP_FCODE].U;
	sprintf((char *)tempStr,"\"V1\": \"%s*%u*%s\",\r",
			"Fcode",
			tmp,
			"N");
	strcat((char *)iEventBuff,(const char *)tempStr);
	tmp=gStuSnapshot.node[tIdx].data[ENU_SNAP_STATE].U;
	sprintf((char *)tempStr,"\"V2\": \"%s*%u*%s\",\r",
			"State",
			tmp,
			"N");
	strcat((char *)iEventBuff,(const char *)tempStr);
	temp=gStuSnapshot.node[tIdx].data[ENU_SNAP_CU_INV1].S;
	sprintf((char *)tempStr,"\"V3\": \"%s*%d*%s\",\r",
			"Iinv1",
			temp,
			"A");
	strcat((char *)iEventBuff,(const char *)tempStr);
	temp=gStuSnapshot.node[tIdx].data[ENU_SNAP_CU_INV2].S;
	sprintf((char *)tempStr,"\"V4\": \"%s*%d*%s\",\r",
			"Iinv2",
			temp,
			"A");
	strcat((char *)iEventBuff,(const char *)tempStr);
	temp=gStuSnapshot.node[tIdx].data[ENU_SNAP_CU_G1].S;
	sprintf((char *)tempStr,"\"V5\": \"%s*%d*%s\",\r",
			"Ig1",
			temp,
			"A");
	strcat((char *)iEventBuff,(const char *)tempStr);
	temp=gStuSnapshot.node[tIdx].data[ENU_SNAP_CU_G2].S;
	sprintf((char *)tempStr,"\"V6\": \"%s*%d*%s\",\r",
			"Ig2",
			temp,
			"A");
	strcat((char *)iEventBuff,(const char *)tempStr);
	temp=gStuSnapshot.node[tIdx].data[ENU_SNAP_VC1].S;
	sprintf((char *)tempStr,"\"V7\": \"%s*%d*%s\",\r",
			"Vc1",
			temp,
			"V");
	strcat((char *)iEventBuff,(const char *)tempStr);
	temp=gStuSnapshot.node[tIdx].data[ENU_SNAP_VC2].S;
	sprintf(tempStr,"\"V8\": \"%s*%d*%s\",\r",
			"Vc2",
			temp,
			"V");
	strcat((char *)iEventBuff,(const char *)tempStr);
	temp=gStuSnapshot.node[tIdx].data[ENU_SNAP_VDC_FIL].S;
	sprintf((char *)tempStr,"\"V9\": \"%s*%d*%s\",\r",
			"VDC_FIL",
			temp,
			"V");
	strcat((char *)iEventBuff,(const char *)tempStr);
	 tmp=gStuSnapshot.node[tIdx].data[ENU_SNAP_STIME].U;
	sprintf(tempStr,"\"V10\": \"%s*%u*%s\",\r",
			"T_Sec",
			tmp,
			"S");
	strcat((char *)iEventBuff,(const char *)tempStr);
	 tmp=gStuSnapshot.node[tIdx].data[ENU_SNAP_UTIME].U;
	sprintf((char *)tempStr,"\"V11\": \"%s*%u*%s\"\r}\r}",
			"T_uSec",
			tmp,
			"uS");
	strcat((char *)iEventBuff,(const char *)tempStr);
	TransmitCMDResponse(iEventBuff);
	gStuSnapshot.tail++;
	}
	else
	{
		TransmitCMDResponse("\r>Event Buffer is Empty\r");
		gStuSnapshot.head=0;
		gStuSnapshot.tail=0;
		gStuSnapshot.id+=1;
	}
}

static void inv_fault_recorder_start_json( u8 *str,DateTime tm)
{

  sprintf((char *)str,"{\r\"serial_number\":\"%s\",\r"
          "\"date\":\"%04d-%02d-%02d\",\r"
            "\"time\":\"%02d:%02d:%02d\",\r"
		  "\"fault\":\"%d\",\r"
		  "\"index\":\"%d\",\r"
            "\"data\":{\r",_serialN,tm.year,tm.month,tm.day,tm.hour,tm.minute,tm.second,gStuSnapshot.id+1,gStuSnapshot.tail+1);
}


u16 inv_fault_recorder_head_report(void)
{
	return gStuSnapshot.head;
}




/*!
 **************************************************************************************************
 *
 *  @fn         u8 debugCommand( char *str)
 *
 *  @par        This function is for handling debug commands.
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
u8 inv_fault_recorder_cmd( char *str)
{
	u8 returnVlaue=1;

	// Check if the command is "on"
	if (strstr((char *)str, "local")) {
		TransmitCMDResponse("\r>Enable Sending Event Log Locally\r");
		gLocalReport=1;
		returnVlaue=0;
	}
	if (strstr((char *)str, "remote")) {
		TransmitCMDResponse("\r>Enable Sending Event Log to Web\r");
		gLocalReport=0;
		returnVlaue=0;
	}

	return returnVlaue;
}
/*!
 **************************************************************************************************
 *
 *  @fn         u8 debugHelp( void)
 *
 *  @par        This function is for handling debug commands help.
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
u8 inv_fault_recorder_cmd_help( void)
{
	static u8 state=0;
	u8 returnVlaue=0;

	return returnVlaue;
}
