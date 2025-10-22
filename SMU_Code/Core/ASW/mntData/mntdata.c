/*
 * mntdata.c
 *
 *  Created on: Jan 14, 2025
 *      Author:
 */
#include "mntdata.h"
#include <stdio.h>
#include <string.h>
#include "dbg.h"

//#include "ASW/MntData/mntdata.h"
float _Vref[31];
static Database_Type mntDb[DATABASE_SIZE];
static size_t dbIndex = 0;
int sendCfg=0;
MntDataType mntData[MONITORING_ARRAY_SIZE]=
{
		{"Live","N",0.0}, //0
		{"VDC","V",0.0},  //1
		{"Iinv1","A",0.0},//2
		{"Iinv2","A",0.0},//3
		{"VO1","V",0.0},  //4  /*Dash Board 1*/
		{"VO2","V",0.0},  //5  /*Dash Board 2*/
		{"VG1","V",0.0},  //6
		{"VG2","V",0.0},  //7
		{"FRQI","Hz",0.0},//8
		{"SSRS","N",0.0},//9  /*Dash Board 3*/
		{"FCODE","N",0.0},//10 /*Dash Board 7*/
		{"FTRIG","N",0.0},//11
		{"Tempinv","C",0.0},//12
		{"InvState","N",0.0},//13
		{"VDC_CH","V",0.0},//14
		{"VBat_CH","V",0.0},//15 /*Dash Board 8*/
		{"IBat_CH","A",0.0},//16
		{"IBRI1C","A",0.0},//17
		{"IBRI2C","A",0.0},//18
		{"POWER1","W",0.0},//19 /*Dash Board 9*/
		{"GenS","N",0.0},//20   /*Dash Board 4*/
		{"State","N",0.0},//21  /*Dash Board 5*/
		{"FCC","N",0.0},//22    /*Dash Board 6*/
		{"TempE","C",0.0},//23
		{"Vtotal","V",0.0},//24
		{"Itotal","A",0.0},//25
		{"SOC","N",0.0},//26
		{"SOH","N",0.0},//27
		{"MaxVcell","V",0.0},//28
		{"Tmax","C",0.0},//29
		{"Tmos","C",0.0},//30
		{"PrtCode","A",0.0},//31
		{"WarCode","A",0.0}//32
};
/*************************************************************/
RefDataType refData[REF_ARRAY_SIZE]=
{
		{"REF1","writeEn","N", 			0.0,	INV_WRITE_EN_INDEX		    	,0}, //0
		{"REF2","sysMode","N",				0.0,	INV_SYS_MOD_INDEX			,0},  //1
		{"REF3","prefGc","W",				0.0,	INV_PREF_GC_INDEX			,0},//2
		{"REF4","wcPrefGC","A",			0.0,	INV_WC_PREF_GC_INDEX		    ,0},//3
		{"REF5","qStar","VAR",				0.0,	INV_Q_STAR_INDEX			,0},  //4
		{"REF6","qUpLimit","VAR",			0.0,	INV_Q_UP_LIMIT_INDEX		,0},//5
		{"REF7","qLowLimit","V",			0.0,	INV_Q_LOW_LIMIT_INDEX		,0},//6
		{"REF8","vBat","V",				0.0,	INV_V_BATT_INDEX			    ,0},  //7
		{"REF9","exVm1","N",				0.0,	INV_EX_VM1_INDEX			,0},//8
		{"REF10","exVm2","N",				0.0,	INV_EX_VM2_INDEX			,0},//9
		{"REF11","exWpll","N",				0.0,	INV_EX_WPLL_INDEX			,0},//10
		{"REF12","vAmpUpLimit","N",		0.0,	INV_V_AMP_UP_LIMIT_INDEX	    ,0},//11
		{"REF13","vAmpLowLimit","V",		0.0,	INV_V_AMP_LOW_LIMIT_INDEX	,0},//12
		{"REF14","vdcStart","V",		0.0,	INV_VDC_START_INDEX			    ,0},//13
		{"REF15","vdcStop","V",			0.0,	INV_VDC_STOP_INDEX			    ,0},//14
		{"REF16","vBattCh","V",			0.0,	INV_VBATT_CH_INDEX			    ,0},//15
		{"REF17","I_Th","A",			0.0,	INV_I_TH_INDEX				    ,0},//16
		{"REF18","rmsOverTime","A",		0.0,	INV_RMS_OVER_TIME_INDEX		    ,0},//17
		{"REF19","I_ThRms","A",			0.0,	INV_RMS_OVER_TIME_INDEX		    ,0},//18
		{"REF20","LoadRlyCtrl","N",		0.0,	INV_LOAD_RLY_INDEX  		    ,0},//19
		{"REF21","Server","N",			0.0,	INV_SERVER_INDEX     		    ,0},//20
		{"REF22","modWriteEn","N",			0.0,	CH_MOD_WRITE_EN_INDEX		,0},//21
		{"REF23","pwrOnUserCmd","N",		0.0,	CH_PWR_ON_USER_CMD_INDEX	,0},//22
		{"REF24","lowBattLowTh","V",		0.0,	CH_LOW_BATT_LOW_TH_INDEX	,0},//23
		{"REF25","lowBattHiTh","V",		0.0,	CH_LOW_BATT_HI_TH_INDEX		    ,0},//24
		{"REF26","powerLowCuttoff","V",	0.0,	CH_PWR_LOW_CUTTOFF_INDEX	    ,0},//25
		{"REF27","lowBattFaultLowTh","V",	0.0,	CH_LOW_BATT_F_LOW_TH_INDEX	,0},//26
		{"REF28","lowBattFaultHiTh","V",	0.0,	CH_LOW_BATT_F_HI_TH_INDEX	,0},//27
		{"REFSM","REFID","N",			0.0,	REFID_INDEX,0}//28
};
MntDataType* getMntData(void){
	return mntData;
}

RefDataType* getRefData(void){
	return refData;
}
Database_Type* getMntDatabase(void){
	return mntDb;
}

float getRefIDValue()
{
	f32 f32ReturnValue=INVALID_DATA;

	for (int i = 0; i < REF_ARRAY_SIZE; i++)
	{
		if (strcmp(refData[i].name, "REFID") == 0)
		{
			f32ReturnValue= refData[i].value;
			break;
		}
	}
	return f32ReturnValue;
}
uint16_t getRefIDIndex()
{
	u16 u16ReturnValue;
	for (int i = 0; i < REF_ARRAY_SIZE; i++)
	{
		if (strcmp(refData[i].name, "REFID") == 0)
		{
			u16ReturnValue=i;
			break;
		}
	}
	return u16ReturnValue;
}
void setRefIDValue(float x)
{
	for (int i = 0; i < REF_ARRAY_SIZE; i++)
	{
		if (strcmp(refData[i].name, "REFID") == 0)
		{
			refData[i].value=x;
			refData[i].flag=(REF_REPORT_TO_WEB | REF_WRITE_TO_MEM | REF_UPDATED_VALUE);
			return;
		}
	}
}

int registerToDatabase(const char *moduleName,
		MntDataType *moduleDb,
		size_t moduleDatasize)
{
	if (dbIndex >= DATABASE_SIZE)
	{
		// Registry is full
		return -1;
	}

	mntDb[dbIndex].mName = moduleName;
	mntDb[dbIndex].mData = moduleDb;
	mntDb[dbIndex].mDataSize = moduleDatasize;
	dbIndex++;

	return 0;
}

int getSizeOfRgsModule(void){
	return dbIndex;
}
/*!
 **************************************************************************************************
 *
 *  @fn         u8 refSetCommand( char *str)
 *
 *  @par        This function is for Set Ref. Values.
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
u8 refSetCommand( char *str)
{
float floatValue;
char ref[10];
u8 u8str[20];
	// Check if the command is "on"
	for(int i=0;i<28;i++)
	{
		snprintf(u8str,10,"%s=",refData[i].ref);
		if(strstr(str,u8str))
				{
			sscanf(str, "%[^=]=%f", ref, &floatValue);
		//	if( refData[i].value!=floatValue)
		//	{
			TransmitCMDResponse( "New Ref received\r");
				refData[i].value=floatValue;
				refData[i].flag=(REF_REPORT_TO_WEB | REF_WRITE_TO_MEM | REF_UPDATED_VALUE);
				setSendCfg(1);
				}

		}
	if(strstr(str,"REFID"))
	{
	float refId=getRefIDValue();
	TransmitCMDResponse( "Set New REFID\r");
				setRefIDValue(refId+1);
				setSendCfg(1);
	}

	return 0;
}
/*!
 **************************************************************************************************
 *
 *  @fn         u8 refSetCommandHelp( void )
 *
 *  @par        Help for This function is for Set Ref. Values.
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
u8 refSetCommandHelp( void )
{
	TransmitCMDResponse( "     ref set = Value        -> New Ref received\r");
	return 0;

}
/*!
 **************************************************************************************************
 *
 *  @fn         void setSendCfg( u8 )
 *
 *  @par        Help for This function is for Set Ref. Values.
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
void setSendCfg( u8 value )
{

	sendCfg=value;
}
/*!
 **************************************************************************************************
 *
 *  @fn         u8 getSendCfg( void )
 *
 *  @par        Help for This function is for Set Ref. Values.
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
u8  getSendCfg( void)
{
	f32 f32RefId=getRefIDValue();
	static f32 staticF32RefIdHold=0;
	static u8 staticU8FirstTime=1;

	if(1==staticU8FirstTime && 0!=f32RefId)
	{
		staticF32RefIdHold=f32RefId;
		staticU8FirstTime=0;
	}
	else
	{
	if(f32RefId!=staticF32RefIdHold)
	{
		staticF32RefIdHold=f32RefId;
		sendCfg=1;
	}
	}
	return sendCfg;
}
