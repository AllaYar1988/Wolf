/*******************
 ***************************************** C SOURCE FILE ******************************************
 *******************/
/**
 *  @file       S2_CH.C.H
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
 *  @li @b      Date            : 2025-05-30
 *  @li @b      Author          : A.Moazami
 *  @li @b      Approved @b by  :
 *  @li @b      Description
 *
 *              Initial Version
 *
 **************************************************************************************************
 */
#include "S2_CH.h"
#include "crc.h"
#include "mntdata.h"
#include "S3_HMI.h"
#include "S5_VGBR.h"
#include "Ctrl.h"
#include "mac.h"
#include "RTE_MB.h"
#include "WebInstanceReport.h"
/*!
 **************************************************************************************************
 *
 *  @fn         Global Private Var Define
 *
 **************************************************************************************************
 */
u16 U16S2UpdateStatus;
S2_CH s2Ch;
MB_Slave_Struct s2Node;
MntDataType s2MntData[S2_MNT_DATA_Size]=
{
		{"VDC_CH","V",0.0},    //0
		{"VBat_CH","V",0.0},   //1 /*Dash Board 8*/
		{"IBat_CH","A",0.0},   //2
		{"IBRI1C","A",0.0},    //3
		{"IBRI2C","A",0.0},    //4
		{"POWER1","W",0.0},    //5 /*Dash Board 9*/
		{"GenS","N",0.0},      //6   /*Dash Board 4*/
		{"State","N",0.0},     //7  /*Dash Board 5*/
		{"FCC","N",0.0},       //8    /*Dash Board 6*/
		{"Ch1REFID","N",0.0},       //9   /*Dash Board 6*/

};
float _memoryMap_bchF[100];
int _countch = 0;
u16 gChTimeout=0;

/*!
 **************************************************************************************************
 *
 *  @fn         Private Function Declaration
 *
 **************************************************************************************************
 */
static float returnChWebRef(int index);
static f32 S2_CH_GetRef(u16 index);
static void S2_CH_Updates(void);
static void S2_CH_resProcess(char *res,int Len);
static int S2_CH_sendReadReq(void);
static int S2_CH_sendWriteReq(void);
static void setChWebRef(u16 u16Index,f32 f32Value);
static void s2_charger_timeout_check(void);
static void s2_charger_timeout_reset(void);

/*!
 **************************************************************************************************
 *
 *  @fn        void S2_CH_registerToMbNode(void)
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
void S2_CH_registerToMbNode(void)
{
	s2Node.id=S2_CH_ID;
	s2Node.senReq=S2_CH_MbMng;
	s2Node.resProcess=S2_CH_resProcess;
	MB_Manager_RegisterSlave(s2Node);

}
/*!
 **************************************************************************************************
 *
 *  @fn         void S2_CH_Database_Init()
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

void S2_CH_Database_Init()
{
	registerToDatabase("S2_CH",
			s2MntData,
			S2_MNT_DATA_Size);
}
/*!
 **************************************************************************************************
 *
 *  @fn         float S2_CH_GetBattPower(void)
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
float S2_CH_GetBattPower(void)
{

	return _memoryMap_bchF[5];
}

/*!
 **************************************************************************************************
 *
 *  @fn         int  S2_CH_MbMng(void)
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
int  S2_CH_MbMng(void)
{
	static int state=READ;
	volatile int status=1;



	S2_CH_Updates();


	switch(state)
	{
	case WRITE:
		if(0==S2_CH_sendWriteReq()){

			s2Ch.writeTimeout=HAL_GetTick();
			state=WRITE_TIMEOUT;
		}
		break;
	case READ:
		if(0==S2_CH_sendReadReq()){
			s2_charger_timeout_check();
			s2Ch.readTimeout=HAL_GetTick();
			state=READ_TIMEOUT;
		}
		break;
	case WRITE_TIMEOUT:
		if((HAL_GetTick()-s2Ch.writeTimeout)>S2_WRITE_TIMEOUT_VALUE){
			status=0;
			state=READ;
		}
		break;
	case READ_TIMEOUT:
		if((HAL_GetTick()-s2Ch.readTimeout)>S2_READ_TIMEOUT_VALUE){
			state=WRITE;
		}
		break;
	}
	return status;
}
/*!
 **************************************************************************************************
 *
 *  @fn         static void S2_CH_resProcess(char *res,int Len)
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
static void S2_CH_resProcess(char *res,int Len)
{ uint32_t chData=0;
int nDb = 0;
int ind = 0;
if (checkFrameCRC(res, Len)) {
	s2_charger_timeout_reset();
	switch (res[1]) {
	case 0x03:

		_countch = 0;
		_memoryMap[235] = (~_memoryMap[235])&0x01;
		nDb = res[2];

		for (int index = 0; index <nDb/4; index++) {
			ind = 4 * index + 3;
			chData = res[ind]<< 24;
			chData|= res[ind + 1] << 16;
			chData|= res[ind + 2] << 8;
			chData|= res[ind + 3];

			memcpy(&_memoryMap_bchF[index], &chData,
					sizeof(float));
		}

		break;
	case 0x10:
		// _memoryMap[234]=0;

		break;
	default:
		break;
	}
}
}

/*!
 **************************************************************************************************
 *
 *  @fn         static int S2_CH_sendReadReq(void))
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
static int S2_CH_sendReadReq(void)
{
	static uint8_t mbStr[20];
	int status = 1;
	static int state=0;

	switch(state)
	{
	case 0:
		mbStr[0] = S2_CH_ID;
		mbStr[1] = READ;
		mbStr[2] = (S2_READ_START_ADDRESS & 0xff00) >> 8;
		mbStr[3] = S2_READ_START_ADDRESS & 0x00ff;
		mbStr[4] = (S2_READ_NUMBER_BYTE & 0xff00) >> 8;
		mbStr[5] = S2_READ_NUMBER_BYTE & 0x00ff;
		uint16_t crc = calculateCRC(mbStr, 6); //
		mbStr[6] = crc & 0x00ff;
		mbStr[7] = (crc & 0xff00) >> 8;
		state=1;
		break;
	case 1:
		if(0==MAC_MbmSendData((uint8_t*)mbStr,8)){
			state=0;
			status=0;
		}
		break;
	}
	return status;
}
/*!
 **************************************************************************************************
 *
 *  @fn        static int S2_CH_sendWriteReq(void)
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
static int S2_CH_sendWriteReq(void)
{
	static uint8_t mbStr[S2_WRITE_BUFFER_SIZE];
	int status = 1;
	static u16 kindex = 7;
	float dummyfloat = 0;
	unsigned long int lintff = 0;



	static int state=0;

	switch(state)
	{
	case 0:
		mbStr[0]= S2_CH_ID;
		mbStr[1] = WRITE;
		mbStr[2] = (S2_WRITE_START_ADDRESS & 0xff00) >> 8;
		mbStr[3] =  S2_WRITE_START_ADDRESS & 0x00ff;
		mbStr[4] = ((S2_WRITE_NUMBER_BYTE+2) & 0xff00) >> 8;
		mbStr[5] =  (S2_WRITE_NUMBER_BYTE+2) & 0x00ff;
		mbStr[6] = 2 * (S2_WRITE_NUMBER_BYTE+2);
		kindex = 7;
		for (int index = 0; index < S2_WRITE_NUMBER_BYTE/2; index++)
		{
			dummyfloat= S2_CH_GetRef(index);
			memcpy(&lintff, &dummyfloat, sizeof(float));

			mbStr[kindex++] = ((lintff) & 0xff00) >> 8;
			mbStr[kindex++] = (lintff) & 0x00ff;
			mbStr[kindex++] = ((lintff) & 0xff000000) >> 24;
			mbStr[kindex++] = ((lintff) & 0xff0000) >> 16;


		}
		S2_CH_setUpdateStatus(1);
		kindex-=4;
		dummyfloat=getRefIDValue();
		memcpy(&lintff, &dummyfloat, sizeof(float));
		mbStr[kindex++] = ((lintff) & 0xff00) >> 8;
		mbStr[kindex++] = (lintff) & 0x00ff;
		mbStr[kindex++] = ((lintff) & 0xff000000) >> 24;
		mbStr[kindex++] = ((lintff) & 0xff0000) >> 16;

		uint16_t crc = calculateCRC(mbStr, kindex); //
		mbStr[kindex++] = crc & 0x00ff;
		mbStr[kindex++] = (crc & 0xff00) >> 8;
		state=1;

		break;
	case 1:
		if(0==MAC_MbmSendData(mbStr,kindex)){
			state=0;
			status=0;
			memset(mbStr,0,100);
		}

		break;
	}
	return status;

}
/*!
 **************************************************************************************************
 *
 *  @fn        static float returnChWebRef(int index)
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
static float returnChWebRef(int index)
{
	RefDataType *rData=getRefData();
	f32 f32ReturnValue=INVALID_DATA;

	if(0!=getRefIDValue())
	{
		for (int i = 0; i < REF_ARRAY_SIZE; i++)
		{
			if ((index+100)==(int)rData[i].index && 1==(rData[i].flag & 0x01))
			{
				f32ReturnValue = rData[i].value;
			}
		}
	}
	return f32ReturnValue;
}
/*!
 **************************************************************************************************
 *
 *  @fn         static void setChWebRef(u16 u16Index,f32 f32Value)
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
static void setChWebRef(u16 u16Index,f32 f32Value)
{
	RefDataType *rData=getRefData();
	u16 u16LocalRemote=_memoryMap[39];
	u16 u16IsAnyCfgSet=(u16)getRefIDValue();

	if(u16LocalRemote>u16IsAnyCfgSet)
	{
		for (int i = 0; i < REF_ARRAY_SIZE; i++)
		{
			if ((u16Index+100)==rData[i].index) {
				if(rData[i].value!=f32Value || (rData[i].flag&REF_UPDATED_VALUE)==0)
				{
					rData[i].flag=(REF_REPORT_TO_WEB | REF_WRITE_TO_MEM | REF_UPDATED_VALUE);
					rData[i].value=f32Value;
				}
			}
		}
	}
}
/*!
 **************************************************************************************************
 *
 *  @fn          void S2_CH_setUpdateStatus(u16 u16Vlaue)
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
void S2_CH_setUpdateStatus(u16 u16Vlaue)
{
	u16 u16LocalRemote=_memoryMap[39];
	u16 u16IsAnyCfgSet=(u16)getRefIDValue();

	if(u16LocalRemote>u16IsAnyCfgSet)
	{
		U16S2UpdateStatus=u16Vlaue;
	}
}
/*!
 **************************************************************************************************
 *
 *  @fn          u16 S2_CH_getUpdateStatus(void)
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
u16 S2_CH_getUpdateStatus(void)
{
	u16 u16LocalRemote=_memoryMap[39];
	u16 u16IsAnyCfgSet=(u16)getRefIDValue();
	u16 u16ReturnValue=0;

	if(u16LocalRemote>u16IsAnyCfgSet)
	{
		u16ReturnValue=U16S2UpdateStatus;
	}
	return u16ReturnValue;
}
/*!
 **************************************************************************************************
 *
 *  @fn        static void S2_CH_Updates(void)
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
static void S2_CH_Updates(void)
{

	Database_Type* mDb=getMntDatabase();
	MntDataType* mData = mDb[1].mData;
	static f32 stcF32HoldData=0;
	//char dstr[50];

	mData = mDb[2].mData;
	mData[S2_MNT_VDC_CH ].value = _memoryMap_bchF[0];
	mData[S2_MNT_VBat_CH].value  = _memoryMap_bchF[1];
	mData[S2_MNT_IBat_CH].value  = _memoryMap_bchF[2];
	mData[S2_MNT_IBRI1C].value  = _memoryMap_bchF[3];
	mData[S2_MNT_IBRI2C].value  = _memoryMap_bchF[4];
	mData[S2_MNT_POWER1].value  = _memoryMap_bchF[5];
	mData[S2_MNT_GenS].value  = _memoryMap_bchF[6];
	mData[S2_MNT_State].value  = _memoryMap_bchF[7];
	mData[S2_MNT_FCC].value  = _memoryMap_bchF[8];
	mData[S2_MNT_REFID].value  = _memoryMap_bchF[9];

   if(mData[S2_MNT_FCC].value!=stcF32HoldData)
    {
	WebInsReportUpdate("FCC",mData[S2_MNT_FCC].value);
	stcF32HoldData=mData[S2_MNT_FCC].value;
	}

	_memoryMap[220] = (int) (_memoryMap_bchF[0] * 10);
	_memoryMap[221] = (int) (_memoryMap_bchF[1] * 10);
	_memoryMap[222] = (int) (_memoryMap_bchF[2] * 10);
	_memoryMap[223] = (int) (_memoryMap_bchF[3] * 10);
	_memoryMap[224] = (int) (_memoryMap_bchF[4] * 10);
	_memoryMap[225] = (int) (_memoryMap_bchF[5]);
	_memoryMap[226] = (int) (_memoryMap_bchF[6] * 10);
	_memoryMap[227] = (int) (_memoryMap_bchF[7] * 10);
	_memoryMap[228] = (int) (_memoryMap_bchF[8] * 10);
	_memoryMap[229] = (int) (_memoryMap_bchF[9] * 10);
	_memoryMap[230] = (int) (_memoryMap_bchF[10] * 10);
	_memoryMap[231] = (int) (_memoryMap_bchF[11] * 10);
}
/*!
 **************************************************************************************************
 *
 *  @fn         static float S1_INV_GetRef(u16 index)
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
static f32 S2_CH_GetRef(u16 index)
{
	f32 f32RefData;
	 u16 u16LocalRemote=_memoryMap[39];
	    u16 u16IsAnyCfgSet=(u16)getRefIDValue();

	if(INVALID_DATA!=returnChWebRef(index)&&
			u16LocalRemote<=u16IsAnyCfgSet)
	{
		f32RefData= returnChWebRef(index);
	}else
	{
		if (index >= 2)
		{
			f32RefData = (float) _memoryMap[index + 19] / 10.0;
		}
		else
		{
			f32RefData = (float) _memoryMap[index + 19];
		}
		setChWebRef(index,f32RefData);
	}



	return f32RefData;
}

static void s2_charger_timeout_check(void)
{
	if( gChTimeout++ == 10)
	{
	for(u16 idx=0;idx<20;idx++)
	 {
		//gArStuMnt[idx].data =0;
		_memoryMap_bchF[idx]=0;
	 }
	}

}

static void s2_charger_timeout_reset(void)
{
	gChTimeout=0;
}
