#include "MdmSrv.h"

#include "ds1307_for_stm32_hal.h"
#include "httpFrame.h"
#include "MAC.h"
#include "dbg.h"
#include "iflash.h"
#include "myrtc.h"
#include "json.h"
#include "mntdata.h"
#include "iwdg.h"
#include <MAC.h>
#include "MdmDll.h"
#include "MdmHw.h"
#include "cjson.h"
#include "WebInstanceReport.h"
#include "server.h"

//#include "MEM.h"
void MX_USART1_UART_Init2(void);
static u8  SIM_init(void);
static u8 StaticU8WebRefIdSts=0;
// 3) We create a single instance of this union, containing all commands
static const ATCommandUnion atCommands = {
		.list = {
				// Index 0
				{ (u8*)"ATE1\r" },
				// Index 1
				{ (u8*)"AT+CCLK?\r" },
				// Index 2
				{ (u8*)"AT+CTZU=1\r" },
				// Index 3
				{ (u8*)"AT+CGATT=1\r" },
				// Index 4
				{ (u8*)"AT+HTTPINIT\r" },
				// Index 5
				/*{ "AT+HTTPPARA=\"URL\",\"https://enerbundsmu.com/api/send-chanel\"\r" },*/
				{ (u8*)"AT+HTTPPARA=\"URL\",\"https://enerbundsmu.com/api/send-chanel\"\r" },
				// Index 6
				{ (u8*)"AT+HTTPPARA=\"URL\",\"https://enerbundsmu.com/api/get-reference\"\r" },
				// Index 7
				{ (u8*)"AT+HTTPPARA=\"CONTENT\",\"application/json\"\r" },
				// Index 8
				{ (u8*)"AT+HTTPPARA=\"CID\",1\r" },
				// Index 9
				{ (u8*)"AT+HTTPACTION=1\r" },
				// Index 10
				{ (u8*)"AT+HTTPREAD=0,300\r" },
				// Index 11
				{ (u8*)"AT+HTTPTERM\r" }
		}
};
// ---------------------------------------------------------------------------
static eSendCommand sendcomm = CMD_ATE;  // Initial state
page_Handletypedef page;

int _readreq=0;
int httpOnOff=0;
int _waittosend=0;
int dataLen=0;


static u16 _refCount=0;
static u8 StcU8MdmSoftRstFlg=0;
u8 aU8Url[20];
enum State {
	IDLE, RECEIVING
} state = IDLE;

GPRS_HandleTypeDef mdmGprs;

int waitnextdata=0;
TIME_OUT timeout;


static void http_Send(GPRS_HandleTypeDef *mGprs);
static void reportHttpSendStatus( );



void ModuleHandle(GPRS_HandleTypeDef *mGprs){

	static char internalState=0;

	switch(internalState)
	{
	case 0:

		internalState=SIM_init();

		break;
	case 1:
		if(0==httpOnOff)
		{
			http_Send(mGprs);
			/*
			if(((HAL_GetTick()-timeout.simSkip)/1000)>30){
				timeout.resetHold=HAL_GetTick();
						internalState=6;
					}
			else
			 */
			if(((HAL_GetTick()-timeout.simHard)/1000)>HARD_RESET_TIMEOUT){
				internalState=2;
			}else if(((HAL_GetTick()-timeout.simSoft)/1000)>SOFT_RESET_TIMEOUT )
			{
				internalState=3;
				StcU8MdmSoftRstFlg=0;
			}
		}

		break;
	case 2:
		TransmitDebug("\rModule Hard Reset...\r");
		MODEM_POWER(OFF)

		timeout.simHard=HAL_GetTick();
		timeout.resetHold=HAL_GetTick();
		internalState=5;
		break;
	case 3:
		timeout.simSoft=HAL_GetTick();
		MDM_SendData(MDM_RESET_CMD);
		TransmitDebug("\rModule Soft Reset...\r");
		internalState=4;
		timeout.resetHold=HAL_GetTick();
		break;
	case 4:
		if(((HAL_GetTick()-timeout.resetHold)/1000)>REST_WAIT_TIME){
			internalState=0;
			sendcomm = CMD_ATE;
		}

		break;
	case 5:
		if(((HAL_GetTick()-timeout.resetHold)/1000)>REST_WAIT_TIME){
			internalState=4;
			timeout.resetHold=HAL_GetTick();
			MODEM_POWER(ON)
		}

		break;
	case 6:

		if(((HAL_GetTick()-timeout.resetHold))>1000)
		{
			MDM_SendData(MDM_SKIP_CMD);
			TransmitDebug("\rModule Escape...\r");
				internalState=7;
				timeout.resetHold=HAL_GetTick();
				timeout.simSkip=HAL_GetTick();
			}

			break;
	case 7:

		if(((HAL_GetTick()-timeout.resetHold))>1000)
		{
				internalState=4;

			}

			break;
	default:

		break;
	}
}
void http_Send(GPRS_HandleTypeDef *mGprs)
{
	static u8  str[200]={0};
	static u16 stcU16Wait=0;
	static u8 stcU8ServerRes=0;
	static int dataLenToRead=0;
			//static int cntRead=0;
	MDMTypeDef* mdm=getMdm();
	static u8 u8HttpUrlFlag=0;

(void)u8HttpUrlFlag;
/*
if(NULL!=strstr(mdm->pData,"AT"))
{
	timeout.simSkip=HAL_GetTick();
}
*/
	switch (sendcomm)
	{
	// -----------------------------------------------------------------------
	case CMD_ATE:  // old "case 1"
	{
		MDM_SendData(atCommands.list[0].command); // "ATE0\r"

		sendcomm = CMD_CGATT; // was "case 2"
	}
	break;

	// -----------------------------------------------------------------------
	case CMD_AT_CCLOCK_REQ: // old "case 101"
	{
		MDM_SendData(atCommands.list[1].command); // "AT+CCLK?\r"
		sendcomm = CMD_CHECK_TIME_TAG; // was "case 102"
	}
	break;

	// -----------------------------------------------------------------------
	case CMD_CHECK_TIME_TAG: // old "case 102"
	{
		//if (_timeTag == 1)
		sendcomm = CMD_CGDCONT; // was "case 3"
		//else
		//	sendcomm = CMD_ATE;      // was "case 1"
	}
	break;

	// -----------------------------------------------------------------------
	case CMD_AT_CTZU: // old "case 100"
	{
		MDM_SendData(atCommands.list[2].command); // "AT+CTZU=1\r"
		sendcomm = CMD_AT_CCLOCK_REQ; // was "case 101"
	}
	break;

	// -----------------------------------------------------------------------
	case CMD_CGATT:  // old "case 2"
	{
		MDM_SendData(atCommands.list[3].command); // "AT+CGATT=1\r"
		sendcomm = CMD_AT_CTZU; // was "case 100"
	}
	break;
	// -----------------------------------------------------------------------
	case CMD_CGDCONT:  // old "case 2"
	{
		sprintf(str,"AT+CGDCONT=1,\"IP\",\"%s\"\r",APN);
		MDM_SendData(str);
		sendcomm = CMD_CGACT; // was "case 100"
	}
	break;
	case CMD_CGACT:  // old "case 2"
	{
		MDM_SendData("AT+CGACT=1,1\r");
		sendcomm = CMD_HTTPINIT; // was "case 100"
	}
	break;
	// -----------------------------------------------------------------------
	case CMD_HTTPINIT: // old "case 3"
	{
		sendcomm = CMD_HTTP_INIT_WAIT; // was "case 6"
		MDM_SendData(atCommands.list[4].command);
		u8HttpUrlFlag=0;
	}
	break;
	case CMD_HTTP_INIT_WAIT:
		sendcomm = CMD_HTTPPARA_CID; // was "case 6"
		break;

	// -----------------------------------------------------------------------
	case CMD_HTTPPARA_URL: // old "case 6"
	{
		memset(str,0,190);
	/*	if (_readreq==0)
		{
			snprintf(str,150,"AT+HTTPPARA=\"URL\",\"%s/api/send-chanel\"\r",URL);
		}
		else if (_readreq == 1)
		{
			snprintf(str,150,"AT+HTTPPARA=\"URL\",\"%s/api/get-reference\"\r",URL);
		}
		else if (_readreq !=1)
		{
			snprintf(str,150,"AT+HTTPPARA=\"URL\",\"%s/api/get-reference?page=%d\"\r",URL,_readreq);
		}*/
		snprintf(str,190,"AT+HTTPPARA=\"URL\",\"%s\"\r",mGprs->api);
		MDM_SendData((uint8_t*)str);
		sendcomm = CMD_HTTPPARA_CONTENT; // was "case 7"

	}
	break;

	// -----------------------------------------------------------------------
	case CMD_HTTPPARA_CONTENT: // old "case 7"
	{
		// index 7 -> "AT+HTTPPARA=\"CONTENT\",\"application/json\"\r"
		MDM_SendData(atCommands.list[7].command);
		sendcomm = CMD_SEND_DATA_LEN; // was "case 8"
	}
	break;

	// -----------------------------------------------------------------------
	case CMD_HTTPPARA_CID: // old "case 8"
	{
		// index 8 -> "AT+HTTPPARA=\"CID\",1\r"
		MDM_SendData(atCommands.list[8].command);
		sendcomm = CMD_HTTPPARA_URL; // was "case 9"
	}
	break;

	// -----------------------------------------------------------------------
	case CMD_SEND_DATA_LEN: // old "case 9"
	{
		if (_readreq==0)
		{


		}
		else
		{
			sprintf((char*)mGprs->sData, "{\r\"serial_number\":\"%s\",\r"
					"\"changed\": \"1\"\r}", _serialN);
		}
		dataLen = strlen((char*)mGprs->sData);
		sprintf(str,"AT+HTTPDATA=%u,10000\r",dataLen);
		MDM_SendData((uint8_t*)str);
		sendcomm = CMD_WAIT_FOR_DOWNLOAD; // was "case 10"
	}
	break;
	case CMD_WAIT_FOR_DOWNLOAD:
		if (stcU16Wait++ > 20 || 0!=strstr((char*)mdm->pData,"DOWNLOAD"))
		{
			stcU16Wait=0;
			sendcomm = CMD_SEND_DATA; // was "case 10"
		}
		break;
	case CMD_SEND_DATA: // old "case 10"
	{
		MDM_SendData((uint8_t*)mGprs->sData);
		sendcomm = CMD_WAIT_FOR_SEND_OK; // was "case 11"
	}
	break;
	case CMD_WAIT_FOR_SEND_OK:
		if (stcU16Wait++ >= 2 || 0!=strstr((char*)mdm->pData,"OK"))
		{
			stcU16Wait=0;
			sendcomm = CMD_HTTP_ACTION; // was "case 10"
		}
		break;
		case CMD_HTTP_ACTION: // old "case 11"
	{
		// index 9 -> "AT+HTTPACTION=1\r"
		MDM_SendData(atCommands.list[9].command);
		sendcomm = CMD_WAIT_RESPONSE; // was "case 109"
	}
	break;

	// -----------------------------------------------------------------------
	case CMD_WAIT_RESPONSE: // old "case 109"
	{
		if (0!=strstr((char*)mdm->pData,"+HTTPACTION: 1,200"))
		{
			timeout.simSoft=HAL_GetTick();
			timeout.simHard=HAL_GetTick();
			StcU8MdmSoftRstFlg=0;
			WebInsReportNextEvent();
			if(mGprs->response!=0)
			{
			char *sstr=strstr((char*)mdm->pData,"+HTTPACTION: 1,200");
			sscanf(sstr,"+HTTPACTION: 1,200,%d",&dataLenToRead);

			sendcomm = CMD_READ_DATA; // was "case 110"
			}else
			{
				sendcomm = CMD_SEND_DATA_LEN; // was "case 17"
				mdmGprs.busy=0;
			}
			stcU16Wait = 0;
			stcU8ServerRes=1;

		}
		else if (0!=strstr((char*)mdm->pData,"+HTTPACTION: 1,5"))
		{
			timeout.simSoft=HAL_GetTick();
			timeout.simHard=HAL_GetTick();
			//char *sstr=strstr((char*)mdm->pData,"+HTTPACTION: 1,500");
			//sscanf(sstr,"+HTTPACTION: 1,500,%d",&dataLenToRead);
			stcU16Wait = 0;
			sendcomm = CMD_HTTP_TERM; // was "case 110"
			stcU8ServerRes=1;

		}
		else if (0!=strstr((char*)mdm->pData,"+HTTPACTION: 1,4"))
		{

			stcU8ServerRes=1;
			sendcomm = CMD_HTTP_TERM; // was "case 110"
					stcU16Wait=0;
		}
		else if (0!=strstr((char*)mdm->pData,"+HTTPACTION: 1,7"))
		{
			sendcomm = CMD_HTTP_TERM; // was "case 110"
			stcU16Wait=0;
		}
		else if(0!=strstr((char*)mdm->pData,"ERROR"))
		{
			sendcomm = CMD_HTTP_TERM; // was "case 110"
			stcU16Wait=0;
		}
		else if(stcU16Wait++ > 10)
		{
			sendcomm = CMD_HTTP_TERM; // was "case 110"
			stcU16Wait=0;
		}

	}
	break;
	case CMD_RESET_MDM: // old "case 110"
		sendcomm = CMD_ATE; // was "case 110"
		StcU8MdmSoftRstFlg=1;

	break;
	// -----------------------------------------------------------------------
	case CMD_READ_DATA: // old "case 110"
	{
		if(dataLenToRead>MIN_DATA_LEN && dataLenToRead<MAX_DATA_LEN)
		{
			snprintf(str,40,"AT+HTTPREAD=0,%d\r\n",dataLenToRead);
			MDM_SendData((uint8_t*)str);//atCommands.list[10].command);
			sendcomm = CMD_WAIT_READ; // was "case 111"
			dataLenToRead=0;
		}else
		{
			sendcomm = CMD_FINISH; // was "case 17"
		}

	}
	break;

	// -----------------------------------------------------------------------
	case CMD_WAIT_READ: // old "case 111"
	{
		static int wtrd = 0;
		if (wtrd++ > 5)
		{
			wtrd = 0;
			sendcomm = CMD_FINISH; // was "case 17"
		}
	}
	break;

	// -----------------------------------------------------------------------
	case CMD_HTTP_TERM: // old "case 12"
	{
		// index 11 -> "AT+HTTPTERM\r"
		MDM_SendData(atCommands.list[11].command);
		sendcomm = CMD_ATE; // was "case 1"
		if(1==stcU8ServerRes)
			{
			mdmGprs.busy=0;
			stcU8ServerRes=0;
			}
	}
	break;

	// -----------------------------------------------------------------------
	case CMD_FINISH: // old "case 17"
	{
		if (0)//_readreq==0)
		{
		sendcomm  = CMD_SEND_DATA_LEN;
		}
		else
		{
			sendcomm  = CMD_HTTPPARA_URL;
		}
		if(1==stcU8ServerRes)
			{
			mdmGprs.busy=0;
			stcU8ServerRes=0;
			}
	}
	break;

	// -----------------------------------------------------------------------
	default:
	{
		// Handle unexpected states if needed
		sendcomm = CMD_ATE; // Reset to known state, for example
	}
	break;
	} // end switch
}
/*!
 **************************************************************************************************
 *
 *  @fn         static u8  SIM_init(void)
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
static u8  SIM_init(void)
{
	static u8 state =0;
	u8 u8ReturnValue=0;

	switch (state)
	{
	case 0:
		MX_USART1_UART_Init();
		state=99;
		break;
	case 99:

		MDM_SendData((uint8_t*)"ATE0\r");
		state=1;
		break;
	case 1:
		MDM_SendData((uint8_t*)"AT+IPR=9600\r");
		state=2;
		break;
	case 2:
		MDM_SendData((uint8_t*)"AT&W\r");
		state=3;
		break;
	case 3:
		MX_USART1_UART_Init2();
		state=4;
		break;
	case 4:
		MDM_SendData((uint8_t*)"ATE1\r");
		state=5;
		break;
	case 5:
		MDM_SendData((uint8_t*)"AT+CSQ\r");
		state=6;
		break;
	case 6:
		MDM_SendData((uint8_t*)"AT+IFC=2,0\r");
		state=7;
		break;
	case 7:
		MDM_SendData((uint8_t*)"AT+CMGF=1\r");
		state=8;
		break;
	case 8:
		MDM_SendData((uint8_t*)"AT+CSMP=49,167,0,0\r");
		state=9;
		break;
	case 9:
		MDM_SendData((uint8_t*)"AT+CSQ\r");
		state=0;
		u8ReturnValue=1;
		break;
	}
	return u8ReturnValue;
}


/*!
 **************************************************************************************************
 *
 *  @fn        int MdmSrv_CheckRefUpdate(uint8_t *str,int *readreq)
 *
 *  @par        This function to check Update Ref Data.
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
int MdmSrv_CheckRefUpdate(uint8_t *str,int *readreq)
{
	int flag=0;
	static u8 staticU8FtWebRefId=1;

	char *start = strstr( (char*)str, "\"count\":");
	if (start == NULL) {
		//TransmitDebug("\rCount field not found\r");
		// return 1;
	}
	else
	{
		DateTime sDt;

		// Use sscanf to extract the date and time values directly
		TransmitDebug("\r\r >> Delivery Report! \r\r");
		// Get statusCode
		if(1==staticU8FtWebRefId)
		{
			StaticU8WebRefIdSts=1;
		staticU8FtWebRefId=0;
		}
		start = strstr((char*)str, "\"count\":");
		if (start) sscanf(start, "\"count\":%u",(unsigned int*) &_refCount);
		if(rtcSetByServer(sDt)==1){
			TransmitDebug("\r\r >> Time Updated by Server! \r\r");
		}
		if(_refCount>getRefIDValue())
		{
			if(0==*readreq){
				TransmitDebug("\r\r >> Check out the new update for references! \r\r");
				*readreq=1;
				flag=1;
			}
		}else
		{

			TransmitDebug("\r\r >> No update for references! \r\r");
		}
	}
	return flag;
}




/*!
 **************************************************************************************************
 *
 *  @fn        static void reportHttpSendStatus(void)
 *
 *  @par        This function is for reporting Http Send status.
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
static void reportHttpSendStatus(void)
{
	char str[50];
	static eSendCommand hState=CMD_ATE;
	if(hState!=sendcomm){
		sprintf(str, ">>R=%d \r", sendcomm);
		//TransmitDebug(str);
		hState=sendcomm;
	}
}
/*!
 **************************************************************************************************
 *
 *  @fn         void mdmResProcess(void)
 *
 *  @par        This function to Process modem Responses.
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
void mdmResProcess(void)
{
	MDMTypeDef* mdm=getMdm();
	//MBTypeDef* mbs=getMbs();
	static int resFlag=0;
	DB2_Toggle


	if(3==MAC_MdmReciveData())//1==process_AT_command())
	{
		resFlag=1;
	}
	if(resFlag==1 )//&& mbs->semaphore==0)
	{
		TransmitDebug((char*)mdm->pData);
		TransmitDebug(".\r");
		resFlag=0;
		MdmSrv_CheckRefUpdate(mdm->pData, &_readreq);
		MdmSrv_UpdateReferences(mdm->pData, &page, &_readreq);
		myrtc_UpdateTime(mdm->pData, &mdt);
	}
}


/*!
 **************************************************************************************************
 *
 *  @fn         int MdmSrv_UpdateReferences(char *str, page_Handletypedef *page, int *readreq)
 *
 *  @par        This function to Update Ref Data.
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
int MdmSrv_UpdateReferences(uint8_t *str, page_Handletypedef *page, int *readreq) {
	int intValue=0;
	float floatValue=0;
	RefDataType *rData=getRefData();
	if(strstr((char *)str,"references")!=NULL)
	{
		intValue=extract_int_value((char *)str,"\"currentPage\"");
		if(intValue!=-1)page->current =intValue;
		intValue=extract_int_value((char *)str,"\"totalPages\"");
		if(intValue!=-1)page->total =intValue;
		if(page->current<=0 || page->current>20 || page->total<=0 || page->total>20)return 0;
		for(int i=0;i<28;i++)
		{
			/*[MOD][Allahyar][based on Name][2025-07-17]*/

			floatValue=extract_value2((char *)str,rData[i].ref);//rData[i].ref
			HAL_IWDG_Refresh(&hiwdg);

			if(floatValue!=INVALID_DATA )
			{
				if(rData[i].value!=floatValue)
				{
				rData[i].value=floatValue;
				rData[i].flag=(REF_WRITE_TO_MEM | REF_UPDATED_VALUE );
				}
				/*[MOD][Allahyar][Return to Web][2025-07-17]*/
				rData[i].flag=(rData[i].flag| REF_REPORT_TO_WEB);
			}


		}

		if (page->current == page->total) {
			*readreq = 0;
			setRefIDValue(_refCount);
		} else {
			*readreq = page->current + 1;
		}
		memset(str,0,BUFFER_SIZE);
	}
	return 0;

}
/*!
 **************************************************************************************************
 *
 *  @fn         u8 MdmSrv_GetWebRefIdSts(void)
 *
 *  @par        This function to get Web RefId status
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
u8 MdmSrv_GetWebRefIdSts(void)
{
	return StaticU8WebRefIdSts;
}
/*!
 **************************************************************************************************
 *
 *  @fn         void MdmSrv_SetWebRefIdSts(u8 u8Value)
 *
 *  @par        This function to set Web RefId status
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
void MdmSrv_SetWebRefIdSts(u8 u8Value)
{
	StaticU8WebRefIdSts=u8Value;
}
/*!
 **************************************************************************************************
 *
 *  @fn         u16 MdmSrv_GetWebRefId(void)
 *
 *  @par        This function to get Web RefId
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
u16 MdmSrv_GetWebRefId(void)
{
	return _refCount;
}



u8 simCommand(char *str)
{
	if(0!=strstr(str,"off"))
	{
		httpOnOff=1;
	}
	else if(0!=strstr(str,"on"))
	{
		httpOnOff=0;
	}
	return 0;
}
u8 simCommandHelp(void)
{
	return 0;
}
u8 atDirectCommand(char *str)
{char sstr[110];
memcpy(sstr,str,100);
MDM_SendData((uint8_t*)sstr);
return 0;
}
u8 atDirectCommandHelp(void)
{
	return 0;
}

void MX_USART1_UART_Init2(void)
{

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 9600;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}
/* USART3 init function */

