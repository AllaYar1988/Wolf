/*******************
 ***************************************** C SOURCE FILE ******************************************
 *******************/
/**
 *  @file       S1_INV.C.H
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
 *  @par        This file create for Control Slave (Inverter)
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


#include "S1_INV.h"
#include "crc.h"
#include "mntdata.h"
#include "S3_HMI.h"
#include "Ctrl.h"
#include "S2_CH.h"
#include "S5_VGBR.h"
#include "mac.h"
#include "RTE_MB.h"
#include "WebInstanceReport.h"
#include "inv_mbmdl_cstm1.h"
#include "inv_fault_recorder.h"



/*!
 **************************************************************************************************
 *
 *  @fn         Global Private Var Define
 *
 **************************************************************************************************
 */
u16 U16S1UpdateStatus=0;
u16 gSnapStatus=0;

static StuCstmModel1 gArStuMnt[ENU_MNT_AR_SIZE];

static StuCstmModel1 gArStuRef[ENU_REF_AR_SIZE];

static int gInvTimeout=0;

static u8 tempTest[250];
u16 gIndexBuffer=0;
u8 gFaultRecorderFag=0;

float tempDummy[50];

S1_INV s1Inv;
MB_Slave_Struct s1Node;

//To report this data to web
MntDataType s1MntData[S1_MNT_DATA_Size]=
{
		{"VDC","V",0.0},  //0
		{"Iinv1","A",0.0},//1
		{"Iinv2","A",0.0},//2
		{"VO1","V",0.0},  //3  /*Dash Board 1*/
		{"VO2","V",0.0},  //4  /*Dash Board 2*/
		{"VG1","V",0.0},  //5
		{"VG2","V",0.0},  //6
		{"FRQI","Hz",0.0},//7
		{"SSRS","N",0.0},//8  /*Dash Board 3*/
		{"FCODE","N",0.0},//9 /*Dash Board 7*/
		{"FTRIG","N",0.0},//10
		{"Tempinv","C",0.0},//11
		{"InvState","N",0.0},//12
		{"Inv1REFID","N",0.0}//12
};
// To get these data from web
MntDataType s1RefData[S1_MNT_DATA_Size]=
{
		{"S1_V1","V",0.0},  //0
		{"S1_V2","A",0.0},//1
		{"S1_V3","A",0.0},//2
		{"S1_V4","V",0.0},  //3
		{"S1_V5","V",0.0},  //4
};
/*!
 **************************************************************************************************
 *
 *  @fn         Private Function Declaration
 *
 **************************************************************************************************
 */
static f32 returnInvWebRef(u16 u16Index);
static void setInvWebRef(u16 u16Index,f32 value);
static void S1_INV_Updates(void);
static f32 S1_INV_GetRef(u16 index);
static void S1_INV_resProcess(char *res,int Len);
static int S1_INV_sendReadReq();
static int S1_INV_sendWriteReq();
static void cstm_mdl1_init(void);
static u16 s1_inverter_read_fault_recorder(void);
static void s1_inverter_timeout_check(void);
static void s1_inverter_timeout_reset(void);



/*!
 **************************************************************************************************
 *
 *  @fn         void S1_INV_registerToMbNode(void)
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
void S1_INV_registerToMbNode(void)
{
	s1Node.id=S1_INV_ID;
	s1Node.senReq=S1_INV_MbMng;
	s1Node.resProcess=S1_INV_resProcess;
	MB_Manager_RegisterSlave(s1Node);

}
/*!
 **************************************************************************************************
 *
 *  @fn         void S1_INV_Database_Init()
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
void S1_INV_Database_Init()
{
	registerToDatabase("S1_INV",
			s1MntData,
			S1_MNT_DATA_Size);
}
/*!
 **************************************************************************************************
 *
 *  @fn         int  S1_INV_MbMng(void)
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
int  S1_INV_MbMng(void)
{
	static int state=READ;
	volatile int status=1;

    u16 temp =inv_fault_recorder_head_report();

	if((gSnapStatus&0xf000)!=0 && temp==0 && gFaultRecorderFag==0)
	{
		TransmitCMDResponse("\r\n> **********  SMU Started Reading Recorded Data From Inverter ********* \r\n");
		gFaultRecorderFag=1;
	}


	if(gFaultRecorderFag==0) // Normal Modbus Read Custom model 1
	{
		S1_INV_Updates();

		switch(state)
		{
		case WRITE:
			if(0==S1_INV_sendWriteReq())
			{
				s1Inv.writeTimeout=HAL_GetTick();
				state=WRITE_TIMEOUT;
			}
			break;
		case READ:
			if(0==S1_INV_sendReadReq())
			{
				s1Inv.readTimeout=HAL_GetTick();
				s1_inverter_timeout_check();
				state=READ_TIMEOUT;
			}
			break;
		case WRITE_TIMEOUT:
			if((HAL_GetTick()-s1Inv.writeTimeout)>S1_WRITE_TIMEOUT_VALUE)
			{
				status=0;
				state=READ;
			}
			break;
		case READ_TIMEOUT:
			if((HAL_GetTick()-s1Inv.readTimeout)>S1_READ_TIMEOUT_VALUE)
			{
				state=WRITE;
			}
			break;
		}
	}
	else   // Read Fault recorder buffer
	{
		switch(state)
		{
		case READ:
			if(0==s1_inverter_read_fault_recorder())
			{
				s1Inv.readTimeout=HAL_GetTick();
				state=READ_TIMEOUT;
			}
			break;
		case READ_TIMEOUT:
			if((HAL_GetTick()-s1Inv.readTimeout)>S1_READ_TIMEOUT_VALUE)
			{
				state=READ;
			}
			break;
		default :
			state=READ;
			break;
		}
	}

	return status;
}
/*!
 **************************************************************************************************
 *
 *  @fn         static void S1_INV_resProcess(char *res,int Len)
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
static void S1_INV_resProcess(char *res,int Len)
{
	u16 invData=0;
	u16 nDb = 0;
	u16 ind = 0;
	u16 mdlId=0;
	u8 nb=0;
	u8 functionCode=0;

	memcpy(tempTest,res,Len);

	if (checkFrameCRC(res, Len)) {
		s1_inverter_timeout_reset();
		nb=res[2];
		functionCode=res[1];
		mdlId=(res[3]<< 8 | res[4]);
		switch(mdlId)
		{
		case MODEL_CSTM_1_ID:

		switch (functionCode) {
		case 0x03:

			_memoryMap[234] = (~_memoryMap[234])&0x01;
			nDb = res[2];
			//gSnapStatus=(res[3]<< 8 | res[4]);
			for (int index =0; index < nDb/2; index++) {
				ind = 2 * index + 3;
				invData = (res[ind    ]<< 8 | res[ind + 1] );
				if(index==2)
				{
					gSnapStatus=invData;
				}

				gArStuMnt[index].data=((float)invData) * gArStuMnt[index].scale;
			}

			break;
		case 0x10:

			break;
		default:
			break;
		}
		break;
		case MODEL_CSTM_2_ID:
		switch (res[1]) {
			case 0x03:

				nDb = res[2];

				for (int index = 1; index < nDb; index++) {
					ind = 2 * index + 3;
					invData = (res[ind    ]<< 8 | res[ind + 1] );
					inv_fault_recorder_set_data(index,invData);
				}
				u8 tmp=inv_fault_recorder_inc_idx();
				if(tmp==0)
				{
					gFaultRecorderFag=0;
					gSnapStatus=0;
				}



				break;
			case 0x10:

				break;
			default:
				break;
			}

			break;
		default:
		break;
		}
	}
}

/*!
 **************************************************************************************************
 *
 *  @fn         static int S1_INV_sendReadReq(void)
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
static int S1_INV_sendReadReq(void)
{
	static uint8_t mbStr[20];
	int status = 1;
	uint16_t crc =0;
	static int state=0;

	switch(state)
	{
	case 0:
		mbStr[0] = S1_INV_ID;
		mbStr[1] = READ;
		mbStr[2] = (S1_READ_START_ADDRESS & 0xff00) >> 8;
		mbStr[3] = S1_READ_START_ADDRESS & 0x00ff;
		mbStr[4] = (S1_READ_NUMBER_BYTE & 0xff00) >> 8;
		mbStr[5] = S1_READ_NUMBER_BYTE & 0x00ff;
		crc = calculateCRC(mbStr, 6); //
		mbStr[6] = crc & 0x00ff;
		mbStr[7] = (crc & 0xff00) >> 8;
		state=1;
		break;
	case 1:
		if(0==MAC_MbmSendData(mbStr,8))
		{
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
 *  @fn         static int s1_inverter_read_fault_recorder(void)
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
static u16 s1_inverter_read_fault_recorder(void)
{
	static uint8_t mbStr[20];
	int status = 1;
	uint16_t crc =0;
	static int state=0;

	switch(state)
	{
	case 0:
		mbStr[0] = S1_INV_ID;
		mbStr[1] = READ;
		mbStr[2] = (S1_READ_FR_START_ADDRESS & 0xff00) >> 8;
		mbStr[3] = S1_READ_FR_START_ADDRESS & 0x00ff;
		mbStr[4] = 0;
		mbStr[5] = gSnapStatus & 0x00ff;
		crc = calculateCRC(mbStr, 6); //
		mbStr[6] = crc & 0x00ff;
		mbStr[7] = (crc & 0xff00) >> 8;
		state=1;
		break;
	case 1:
		if(0==MAC_MbmSendData(mbStr,8))
		{
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
 *  @fn         static int S1_INV_sendWriteReq(void)
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
static int S1_INV_sendWriteReq(void)
{
	static uint8_t mbStr[S1_WRITE_BUFFER_SIZE];
	int status = 1;
	static int kindex = 7;
	int temp=0;
	uint16_t crc = 0;
	static int state=0;



	switch(state)
	{
	case 0:
		mbStr[0]= S1_INV_ID;
		mbStr[1] = WRITE;
		mbStr[2] = (S1_WRITE_START_ADDRESS & 0xff00) >> 8;
		mbStr[3] =  S1_WRITE_START_ADDRESS & 0x00ff;
		mbStr[4] = (S1_WRITE_NUMBER_BYTE & 0xff00) >> 8;
		mbStr[5] =  S1_WRITE_NUMBER_BYTE & 0x00ff;
		mbStr[6] = 2 * S1_WRITE_NUMBER_BYTE;
		kindex = 7;
		for (int index = 0; index < S1_WRITE_NUMBER_BYTE/2; index++)
		{
			//static float PrefPrev = -0.2;
			temp = (int)(S1_INV_GetRef(index));
			tempDummy[index]=temp;
			mbStr[kindex++] = ((temp) & 0xff00) >> 8;
			mbStr[kindex++] = (temp) & 0x00ff;

		}
		S1_INV_setUpdateStatus(1);

		crc = calculateCRC(mbStr, kindex); //
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
 *  @fn         static f32 returnInvWebRef(u16 u16Index)
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
static f32 returnInvWebRef(u16 u16Index)
{
	RefDataType *rData=getRefData();
    f32 f32ReturnValue=INVALID_DATA;
    u16 u16IsAnyCfgSet=(u16)getRefIDValue();

if(0!=u16IsAnyCfgSet)
{
	for (int i = 0; i < REF_ARRAY_SIZE; i++)
	{
		if (u16Index==rData[i].index && REF_UPDATED_VALUE==(rData[i].flag & REF_UPDATED_VALUE)) {
			f32ReturnValue= rData[i].value;
		}
	}
}
return f32ReturnValue;
}
/*!
 **************************************************************************************************
 *
 *  @fn         static float returnInvWebRef(int index)
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
static void setInvWebRef(u16 u16Index,f32 f32Value)
{
	RefDataType *rData=getRefData();
    u16 u16LocalRemote=_memoryMap[39];
    u16 u16IsAnyCfgSet=(u16)getRefIDValue();

if(u16LocalRemote>u16IsAnyCfgSet)
{
	for (int i = 0; i < REF_ARRAY_SIZE; i++)
	{
		if (u16Index==rData[i].index) {
			 if(rData[i].value!=f32Value ||
					 ((rData[i].flag)&REF_UPDATED_VALUE)!=REF_UPDATED_VALUE)
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
 *  @fn          void S1_INV_setUpdateStatus(u16 u16Vlaue)
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
 void S1_INV_setUpdateStatus(u16 u16Vlaue)
{
    u16 u16LocalRemote=_memoryMap[39];
    u16 u16IsAnyCfgSet=(u16)getRefIDValue();

if(u16LocalRemote>u16IsAnyCfgSet)
{
	U16S1UpdateStatus=u16Vlaue;
}
}
/*!
 **************************************************************************************************
 *
 *  @fn          void S1_INV_getInvUpdate(void)
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
 u16 S1_INV_getUpdateStatus(void)
{
    u16 u16LocalRemote=_memoryMap[39];
    u16 u16IsAnyCfgSet=(u16)getRefIDValue();
    u16 u16ReturnValue=0;

if(u16LocalRemote>u16IsAnyCfgSet)
{
	u16ReturnValue=U16S1UpdateStatus;
}
return u16ReturnValue;
}
/*!
 **************************************************************************************************
 *
 *  @fn         static void S1_INV_Updates(void)
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
static void S1_INV_Updates(void)
{
	Database_Type* mDb=getMntDatabase();
	MntDataType* mData = mDb[1].mData;
    static f32 stcF32HoldData=0;
    static u8 firstTime=1;

    if(firstTime==1)
    {
    	 cstm_mdl1_init();
         firstTime=0;
    }
	mData[S1_MNT_VDC     ].value  = gArStuMnt[ENU_MNT_VDC_AVG].data;
	mData[S1_MNT_Iinv1   ].value  = gArStuMnt[ENU_MNT_CU_INV1_RMS].data;
	mData[S1_MNT_Iinv2   ].value  = gArStuMnt[ENU_MNT_CU_INV2_RMS].data ;
	mData[S1_MNT_VO1     ].value  = gArStuMnt[ENU_MNT_VC1_RMS].data;
	mData[S1_MNT_VO2     ].value  = gArStuMnt[ENU_MNT_VC2_RMS].data ;
	mData[S1_MNT_VG1     ].value  = gArStuMnt[ENU_MNT_VG1_RMS].data;
	mData[S1_MNT_VG2     ].value  = gArStuMnt[ENU_MNT_VG2_RMS].data;
	mData[S1_MNT_FRQI    ].value  = gArStuMnt[ENU_MNT_FRQ_INV].data;
	mData[S1_MNT_SSRS    ].value  = gArStuMnt[ENU_MNT_SSR_STS].data;
	mData[S1_MNT_FCODE   ].value  = gArStuMnt[ENU_MNT_FCODE].data;
	mData[S1_MNT_FTRIG   ].value  = gArStuMnt[ENU_MNT_FAULT_TRIG].data;
	mData[S1_MNT_Tempinv ].value  = gArStuMnt[ENU_MNT_TEMP2].data ;
	mData[S1_MNT_InvState].value  = gArStuMnt[ENU_MNT_STATE].data;
	//gSnapStatus=gArStuMnt[ENU_MNT_SNAP].data;
	mData[S1_MNT_REFID   ].value  = 0;//_memoryMap_invF[17];

	if(mData[S1_MNT_FCODE].value!=stcF32HoldData)
	{
		WebInsReportUpdate("FCODE",mData[S1_MNT_FCODE].value);
		stcF32HoldData=mData[S1_MNT_FCODE].value;

	}

	for(u16 idx=3;idx<ENU_MNT_AR_SIZE;idx++)
	{

		_memoryMap[200+idx-2]=(u16) (gArStuMnt[idx].data / gArStuMnt[idx].scale);
	}


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
static f32 S1_INV_GetRef(u16 index)
{
	float f32RefData;
	f32 f32RefValueFromMemoryMap=(f32)_memoryMap[index];
	Database_Type *mDb = getMntDatabase();
	u16 u16LocalRemote=_memoryMap[39];
	u16 u16IsAnyCfgSet=(u16)getRefIDValue();

	f32RefData=f32RefValueFromMemoryMap;

	if(INVALID_DATA!=returnInvWebRef(index) &&
			u16LocalRemote<=u16IsAnyCfgSet)
	{
		f32RefData= returnInvWebRef(index)*gArStuRef[index].scale;
	}

	if (index == ENU_REF_VBAT_CH)
	{

		f32RefData = mDb[2].mData[S2_MNT_VBat_CH].value*gArStuRef[index].scale;

	}

	return f32RefData;
}



static void cstm_mdl1_init(void)
{
	/* User-supplied code: Operation 'cstm_mdl1_init', {6B5524CA-0C8F-4962-B3A8-2A770462946E} */
	/* SyncableUserCode{6B5524CA-0C8F-4962-B3A8-2A770462946E}:1QpzFfbg6f */
	gArStuMnt[ENU_MNT_ID].scale = 0.1;
	gArStuMnt[ENU_MNT_LEN].scale = 0.1;
	gArStuMnt[ENU_MNT_SNAP].scale = 0.1;

	gArStuMnt[ENU_MNT_VDC_AVG].scale = 0.1;
	gArStuMnt[ENU_MNT_CU_INV1_RMS].scale = 0.1;
	gArStuMnt[ENU_MNT_CU_INV2_RMS].scale = 0.1;
	gArStuMnt[ENU_MNT_VC1_RMS].scale = 0.1;
	gArStuMnt[ENU_MNT_VC2_RMS].scale = 0.1;
	gArStuMnt[ENU_MNT_VG1_RMS].scale = 0.1;
	gArStuMnt[ENU_MNT_VG2_RMS].scale = 0.1;
	gArStuMnt[ENU_MNT_FRQ_INV].scale = 0.1;
	gArStuMnt[ENU_MNT_FRQ_GRID].scale = 0.1;
	gArStuMnt[ENU_MNT_SSR_STS].scale = 1;
	gArStuMnt[ENU_MNT_FCODE].scale = 1;
	gArStuMnt[ENU_MNT_FAULT_TRIG].scale = 1;
	gArStuMnt[ENU_MNT_FAULT_FLAG].scale = 1;
	gArStuMnt[ENU_MNT_TEMP1].scale = 0.1;
	gArStuMnt[ENU_MNT_TEMP2].scale = 0.1;
	gArStuMnt[ENU_MNT_VBAT_FIL].scale = 0.1;
	gArStuMnt[ENU_MNT_STATE].scale = 1;
	gArStuMnt[ENU_MNT_SNAP].scale = 1;



     gArStuRef[ENU_REF_WRITE_EN].scale = 10;
     gArStuRef[ENU_REF_SYS_MODE].scale = 10;
     gArStuRef[ENU_REF_PREF_GC].scale = 10;
     gArStuRef[ENU_REF_RATE_PREF_GC].scale = 100;
     gArStuRef[ENU_REF_Q_STAR].scale = 10;
     gArStuRef[ENU_REF_Q_UP_LMT].scale = 10;
     gArStuRef[ENU_REF_Q_LW_LMT].scale = 10;
     gArStuRef[ENU_REF_VBF].scale = 10;
     gArStuRef[ENU_REF_EXT_VM1].scale = 10;
     gArStuRef[ENU_REF_EXT_VM2].scale = 10;
     gArStuRef[ENU_REF_EXT_WPLL].scale = 10;
     gArStuRef[ENU_REF_V_AMP_UPLMT].scale = 10;
     gArStuRef[ENU_REF_V_AMP_LWLMT].scale = 10;
     gArStuRef[ENU_REF_VDC_START].scale = 10;
     gArStuRef[ENU_REF_VDC_STOP].scale = 10;
     gArStuRef[ENU_REF_VBAT_CH].scale = 10;
     gArStuRef[ENU_REF_I_TH].scale = 10;
     gArStuRef[ENU_REF_RMS_OVT].scale = 10;
     gArStuRef[ENU_REF_I_TH_RMS].scale = 10;
    /* SyncableUserCode{6B5524CA-0C8F-4962-B3A8-2A770462946E} */

}

static void s1_inverter_timeout_check(void)
{
	if( gInvTimeout++ == 10)
	{
	for(u16 idx=0;idx<ENU_MNT_AR_SIZE;idx++)
	 {
		gArStuMnt[idx].data =0;
	 }
	}

}

static void s1_inverter_timeout_reset(void)
{
      gInvTimeout=0;
}
