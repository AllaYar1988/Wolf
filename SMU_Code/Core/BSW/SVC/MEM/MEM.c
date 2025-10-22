/*******************
 ***************************************** C SOURCE FILE ******************************************
 *******************/
/**
 *  @file       MEM.C.H
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
 *  @par        This file create for Memory Service
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
#include "MEM.h"

#include "fatfs.h"
#include "iflash.h"
#include <string.h>
#include "dbg.h"
#include "crc.h"
#include "myrtc.h"
#include "json.h"
#include "mntdata.h"
#include "WCET.h"

/*!
 **************************************************************************************************
 *
 *  @fn         Global Private Var Define
 *
 **************************************************************************************************
 */
FATFS SDFatFs;  /* File system object for SD card logical drive */
FIL  MyFile;    /* File object */
SD_HAndle_Type sdFile;
u32 U32ReadWcTime=0;
u32 U32MkDirWcTime=0;
u32 U32WriteWcTime=0;
/*!
 **************************************************************************************************
 *
 *  @fn         Private Function Declaration
 *
 **************************************************************************************************
 */
#define REF_BUFFER_SIZE 64
#define FILE_PATH_SIZE 20
int SD_init(char immidiate);
static void SD_Error(const char *state, char *error, int rc);
u8 readConfigFile(int index);
u8 updateConfig(int index);

/*!
 **************************************************************************************************
 *
 *  @fn         static void SD_Error(const char *state, char *error, int rc)
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
static void SD_Error(const char *state, char *error, int rc)
{
	// Array of string literals representing possible results
	const char *Result[] = {
			"OK", //0
			"DISK_ERR",//1
			"INT_ERR", //2
			"NOT_READY", //3
			"NO_FILE", //4
			"NO_PATH",//5
			"INVALID_NAME", //6
			"DENIED", //7
			"EXIST", //8
			"INVALID_OBJECT", //9
			"WRITE_PROTECTED",//10
			"INVALID_DRIVE", //11
			"NOT_ENABLED", //12
			"NO_FILE_SYSTEM", //13
			"MKFS_ABORTED", //14
			"TIMEOUT",//15
			"LOCKED", //16
			"NOT_ENOUGH_CORE",//17
			"TOO_MANY_OPEN_FILES"//18
	};

	// Calculate the number of elements in the Result array
	const int numResults = sizeof(Result) / sizeof(Result[0]);

	// Ensure the rc is within the valid range
	if (rc >= 0 && rc < numResults) {
		snprintf(error, MAX_BUFFER_SIZE, ">> %s.SD Status.%s\r", state, Result[rc]);
	} else {
		snprintf(error, MAX_BUFFER_SIZE, ">> %s.SD Status.INVALID_RESULT_CODE\r", state);
	}
	TransmitDebug(error);

}
/*!
 **************************************************************************************************
 *
 *  @fn         void memInit()
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
void memInit()
{
	SD_init(0);
}
/*!
 **************************************************************************************************
 *
 *  @fn         void memHnadler(int index,MEM_STATES state)
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
u8 memHnadler(u16 u16Index,MEM_STATES state)
{
	//static MEM_STATES iState=MEM_INIT;
	u16 u16Error=0;
	u8 u8ReturnValue=0;
	u8 u8InternalState=state;
	// iState=state;

	switch(u8InternalState)
	{
	case MEM_INIT:
		WCET_SetDwt(0);
		// WCET = (16924788/168000000) = 100 msec
		u16Error=f_mkdir("CFG");
		WCET_Update(&U32MkDirWcTime);
		//iState=MEM_IDLE;
		if(0==u16Error)
		{
			u8ReturnValue=1;
		}
		else if(5==u16Error || 1==u16Error)
		{
			u8ReturnValue=2;
		}
		else
		{
			u8ReturnValue=1;
		}
		break;
	case MEM_IDLE:
		//iState=state;
		break;
	case MEM_SAVE_CONFIG:
		WCET_SetDwt(0);
			u8ReturnValue=updateConfig(u16Index);
		WCET_Update(&U32WriteWcTime);
		//iState=MEM_IDLE;
		break;
	case MEM_READ_CONFIG:
		WCET_SetDwt(0);
			u8ReturnValue=readConfigFile(u16Index);
		WCET_Update(&U32ReadWcTime);

		//iState=MEM_IDLE;
		break;
	}
	return u8ReturnValue;
}
/*!
 **************************************************************************************************
 *
 *  @fn         void readConfigFile(int index)
 *
 *  @par
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : (487755/168000000) = 2.9 msec
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
u8 readConfigFile(int index)
{
	char eR[MAX_BUFFER_SIZE];
	char buffer[REF_BUFFER_SIZE];
	static char filePath[FILE_PATH_SIZE];
	float floatValue=0;
	UINT bytes_Read=0;
	static FIL  myFileConfig;
	RefDataType *rData=getRefData();
	u8 u8ReturnValue=0;
	static u8 u8State=0;
	u16 u16Error=0;

	switch(u8State)
	{
	case 0:
		snprintf(filePath, FILE_PATH_SIZE, "CFG/%s.txt",rData[index].ref);
		u16Error=f_open(&myFileConfig,filePath,FA_WRITE|FA_READ|FA_OPEN_EXISTING);

        if(0==u16Error)
		{
			u8State=1;
		}
		else
		{
			u8State=0;
			u8ReturnValue=1;
		}
		SD_Error("Open",eR,u16Error);
		break;
	case 1:

		f_lseek(&myFileConfig,0);
		u16Error=f_read(&myFileConfig, buffer, REF_BUFFER_SIZE, &bytes_Read);

		SD_Error("Read",eR,u16Error);
		if(0==u16Error)
		{
			floatValue=extract_value(buffer,rData[index].name);
			rData[index].value=floatValue;
			rData[index].flag=(REF_REPORT_TO_WEB | REF_UPDATED_VALUE);
			char buffer2[REF_BUFFER_SIZE+5];
			snprintf(buffer2,REF_BUFFER_SIZE+1,"%s\r",buffer);
			TransmitCMDResponse(buffer2);
			u8State=2;
		}
		else
		{
			u8State=2;
		}
		break;
	case 2:

		u16Error=f_close(&myFileConfig);
				if(0==u16Error)
				{
					u8State=0;
					u8ReturnValue=1;
				}
				else
				{
					u8State=0;
					u8ReturnValue=1;
				}
				SD_Error("Close",eR,u16Error);
				break;
	}
	return u8ReturnValue;
}
/*!
 **************************************************************************************************
 *
 *  @fn         void updateConfig(int index)
 *
 *  @par
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : (1380110 / 168000000) = 8.2 msec
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
u8 updateConfig(int index)
{
	char eR[MAX_BUFFER_SIZE];
	char buffer[REF_BUFFER_SIZE];
	static char filePath[FILE_PATH_SIZE];
	UINT bytes_written=0;
	static FIL  myFileConfig;
	RefDataType *rData=getRefData();
	u8 u8ReturnValue=0;
	static u8 u8State=0;
	u16 u16Error=0;

	switch(u8State)
	{
	case 0:
		snprintf(filePath, FILE_PATH_SIZE, "CFG/%s.txt",rData[index].ref);
		u16Error=f_open(&myFileConfig,filePath,FA_WRITE|FA_READ|FA_OPEN_EXISTING);
		if(0==u16Error)
		{
			u8State=2;
		}
		else if(4==u16Error)
		{
			u8State=1;
		}
		else if(5==u16Error || 1==u16Error)
		{
			u8State=0;
			u8ReturnValue=2;
		}
		else
		{
			u8State=0;
			u8ReturnValue=1;
		}
		SD_Error("Open",eR,u16Error);
		break;
	case 1:
		/*_________________________________________*/
		/*   If There is No File Create a New One  */
		u16Error=f_open(&myFileConfig,filePath,FA_WRITE|FA_CREATE_NEW|FA_READ);
		if(0==u16Error)
		{
			u8State=2;
		}
		else
		{
			u8State=0;
			u8ReturnValue=1;
		}
		SD_Error("Create",eR,u16Error);
		break;
	case 2:
		memset(buffer,' ',REF_BUFFER_SIZE);
		snprintf(buffer, REF_BUFFER_SIZE,"\"%s\": \"%s*%0.1f*%s\"", rData[index].ref,
				rData[index].name,rData[index].value,rData[index].unit);
		f_lseek(&myFileConfig,0);
		u16Error=f_write(&myFileConfig, buffer, REF_BUFFER_SIZE, &bytes_written);
		if(0==u16Error)
		{
			u8State=3;
			char buffer2[REF_BUFFER_SIZE+5];
			snprintf(buffer2,REF_BUFFER_SIZE+1,"%s\r",buffer);
			TransmitCMDResponse(buffer2);
			u8ReturnValue=3;
		}
		else
		{
			u8State=3;
		}
		SD_Error("Write",eR,u16Error);
		break;
	case 3:
		f_sync(&myFileConfig); // Forces write to SD
		u8State=4;
		break;
	case 4:
     	u16Error=f_close(&myFileConfig);
		if(0==u16Error)
		{
			u8State=0;
			u8ReturnValue=4;
		}
		else
		{
			u8State=4;
			u8ReturnValue=1;
		}
		SD_Error("Close",eR,u16Error);
		break;
	}
	return u8ReturnValue;
}
/*!
 **************************************************************************************************
 *
 *  @fn         static int SD_init(char immidiate)
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
int SD_init(char immidiate)
{
	char Error;

	Error = f_mount(&SDFatFs, (TCHAR const*)SDPath, immidiate);

	return Error;
}


/*!
 **************************************************************************************************
 *
 *  @fn         u8 MEM_SdStatusCheck(void);
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
u8 MEM_SdStatusCheck(void)
{
	static u8 staticU8SdDetectStatus=0;
	u8 u8ReturnValue=0;

	if(GET_SD_DETECT==1)
	{
		//u8InternalState=MEM_IDLE;
		u8ReturnValue=1;
		if(0==staticU8SdDetectStatus)
		{
			TransmitCMDResponse("Please Insert SD Memory Card!\r");
		staticU8SdDetectStatus=1;
		}
	}else
	{
		if(1==staticU8SdDetectStatus)
		{
		TransmitCMDResponse("SD Memory Card Inserted!\r");
		staticU8SdDetectStatus=0;
		u8ReturnValue=2;
		}
	}
	return u8ReturnValue;
}
