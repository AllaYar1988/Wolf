/*******************
 ***************************************** C SOURCE FILE ******************************************
 *******************/
/**
 *  @file       MAC.C
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
 *  @li @b      Date            : 2025-05-04
 *  @li @b      Author          : A.Moazami
 *  @li @b      Approved @b by  :
 *  @li @b      Description
 *
 *              Initial Version
 *
 **************************************************************************************************
 */
#include "MAC.h"
#include "modbus.h"
#include "dbg.h"
#include "Platform.h"

MBTypeDef mbs;
MBTypeDef mbm;

MBTypeDef* getMbs(void)
{
	return &mbs;
}

MBTypeDef* getMbm(void)
{
	return &mbm;
}


/*!
 **************************************************************************************************
 *
 *  @fn         static void MAC_MbmSendData (void)
 *
 *  @par        This function Sends MBM data.
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
int MAC_MbmSendData(uint8_t *mbStr,int mbDataLen)
{
	int status=1;
	static int state = 0;
	static u16 stcU16Timeout=0;


	switch(state)
	{
	case 0:
		stcU16Timeout=0;
		__SIO_Master(GPIO_PIN_SET);
		MbmUart.gState = HAL_UART_STATE_READY;

		HAL_UART_Transmit_DMA(&MbmUart,mbStr,mbDataLen);
		state=1;
		break;
	case 1:
		if (__HAL_UART_GET_FLAG(&MbmUart, UART_FLAG_TC) != RESET)
		{
			__SIO_Master(GPIO_PIN_RESET);
			state =0;
			status=0;
			stcU16Timeout=0;
		}
		break;
	}
	if(stcU16Timeout++>2000)
	{
		stcU16Timeout=0;
		__SIO_Master(GPIO_PIN_RESET);
			state =0;
			status=0;
	}

	return status;
}

/*!
 **************************************************************************************************
 *
 *  @fn         static void MAC_MbsSendData (void)
 *
 *  @par        This function Sends MBS data.
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
int MAC_MbsSendData(void)
{

	int status=1;
	static int state = 0;
	static u16 stcU16Timeout=0;

	if(1==mbs.semaphore)
	{
		switch(state)
		{
		case 0:
			//DB2_SET_PIN
			__SIO_SLAVE(GPIO_PIN_SET)
			MbsUart.gState = HAL_UART_STATE_READY;

			HAL_UART_Transmit_DMA(&MbsUart,mbs.sData,mbs.dataLen);
			state=1;
			break;
		case 1:
			if (__HAL_UART_GET_FLAG(&MbsUart, UART_FLAG_TC) != RESET)
			{
				__SIO_SLAVE(GPIO_PIN_RESET)
				state =0;
				status=0;
				mbs.semaphore=0;
			}
			break;
		}
		if(stcU16Timeout++>1000)
		{
			stcU16Timeout=0;
			__SIO_SLAVE(GPIO_PIN_RESET)
				state =0;
				status=0;
				mbs.semaphore=0;
		}
	}
	else
	{
		stcU16Timeout=0;
	}
	return status;
}

/*!
 **************************************************************************************************
 *
 *  @fn         static void MAC_MailToMbs (void)
 *
 *  @par        This function mail data to MBS.
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
void MAC_MailToMbs(char *mbStr,int mbDataLen)
{
	if(0!=mbDataLen && 0==mbs.semaphore){
		if(mbDataLen<MBS_DMA_SEND_BUF_SIZE)
		{
			memcpy(mbs.sData,mbStr,mbDataLen);
			mbs.semaphore=1;
			mbs.dataLen=mbDataLen;
		}
	}
}

/*!
 **************************************************************************************************
 *
 *  @fn         static void MAC_MbsReciveData (void)
 *
 *  @par        This function Receives MBS data.
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
int MAC_MbsReciveData(void)
{
	static int state=0;
	int rtnValue=0;
	switch(state)
	{
	case 0:
		HAL_UART_Receive_DMA(&MbsUart, (uint8_t*) mbs.rData, MBS_DMA_BUF_SIZE);
		state=1;
		break;
	case 1:
		if (__HAL_UART_GET_FLAG(&MbsUart, UART_FLAG_IDLE))
		{
			uint32_t tmp;
			// Clear the idle flag by reading the status register and then the data register.
			// This prevents the flag from retriggering immediately.
			HAL_UART_DMAStop(&MbsUart);
			tmp = MbsUart.Instance->SR;
			tmp = MbsUart.Instance->DR;
			(void)tmp; // Avoid compiler warning about unused variable
			memcpy(mbs.pData,mbs.rData,MBS_DMA_BUF_SIZE);
			memset(mbs.rData,0,MBS_DMA_BUF_SIZE);
			mbs.byteCount= MBS_DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(MbsUart.hdmarx);
			rtnValue=3;
			HAL_UART_Receive_DMA(&MbsUart, (uint8_t*) mbs.rData, MBS_DMA_BUF_SIZE); //GSM
		}
		break;
	default:
		break;
	}
	return rtnValue;
}

/*!
 **************************************************************************************************
 *
 *  @fn         static void MAC_MbmReciveData (void)
 *
 *  @par        This function Receives MBM data.
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
int MAC_MbmReciveData(void)
{
	static int state=0;
	int rtnValue=0;
	switch(state)
	{
	case 0:
		HAL_UART_Receive_DMA(&MbmUart, (uint8_t*) mbm.rData, MBM_DMA_BUF_SIZE);

		state=1;
		break;
	case 1:
		if (__HAL_UART_GET_FLAG(&MbmUart, UART_FLAG_IDLE))
		{
			uint32_t tmp;

			// Clear the idle flag by reading the status register and then the data register.
			// This prevents the flag from retriggering immediately.
			HAL_UART_DMAStop(&MbmUart);
			tmp = MbmUart.Instance->SR;
			tmp = MbmUart.Instance->DR;
			(void)tmp; // Avoid compiler warning about unused variable
			memcpy(mbm.pData,mbm.rData,MBM_DMA_BUF_SIZE);
			memset(mbm.rData,0,MBM_DMA_BUF_SIZE);
			mbm.byteCount= MBM_DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(MbmUart.hdmarx);
			rtnValue=3;
			HAL_UART_Receive_DMA(&MbmUart, (uint8_t*) mbm.rData, MBM_DMA_BUF_SIZE); //GSM
		}
		break;
	default:
		break;
	}
	return rtnValue;
}
