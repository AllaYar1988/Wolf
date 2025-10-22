/*
 * MdmDll.c
 *
 *  Created on: Apr 8, 2025
 *      Author: 98912
 */


#include "MdmDll.h"
#include "dbg.h"
#include "platform.h"
MDMTypeDef mdm;
u8 U8MdmBuffer[500];
MDMTypeDef* getMdm(void)
{
	return &mdm;
}

/*!
 **************************************************************************************************
 *
 *  @fn         static void serialmodbus (void)
 *
 *  @par        This function Sends MDM data.
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
void MDM_SendData(uint8_t *str)
{
	//MdmUart.gState = HAL_UART_STATE_READY;
	int size=strlen((char *)str);
	memset(U8MdmBuffer,0,500);
	strncpy(U8MdmBuffer,str,size);
	MdmUart.gState = HAL_UART_STATE_READY;
	HAL_UART_Transmit_DMA(&MdmUart,U8MdmBuffer,size);
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
int MAC_MdmReciveData(void)
{
	static int state=0;
	int rtnValue=0;
	HAL_GPIO_WritePin(uCTSM_GPIO_Port,uCTSM_Pin,GPIO_PIN_RESET);
	switch(state)
	{
	case 0:
		HAL_UART_Receive_DMA(&MdmUart, (uint8_t*) mdm.rData, MDM_DMA_BUF_SIZE);
		state=1;
		break;
	case 1:
		if (__HAL_UART_GET_FLAG(&MdmUart, UART_FLAG_IDLE))
		{

			HAL_GPIO_WritePin(uCTSM_GPIO_Port,uCTSM_Pin,GPIO_PIN_SET);

			uint32_t tmp;
			// Clear the idle flag by reading the status register and then the data register.
			// This prevents the flag from retriggering immediately.
			HAL_UART_DMAStop(&MdmUart);
			tmp = MdmUart.Instance->SR;
			tmp = MdmUart.Instance->DR;
			(void)tmp; // Avoid compiler warning about unused variable
			memcpy(mdm.pData,mdm.rData,MDM_DMA_BUF_SIZE);
			memset(mdm.rData,0,MDM_DMA_BUF_SIZE);
			//mdm.byteCount= MDM_DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(MdmUart.hdmarx);
			rtnValue=3;
			HAL_UART_Receive_DMA(&MdmUart, (uint8_t*) mdm.rData, MDM_DMA_BUF_SIZE); //GSM
		}
		break;
	default:
		break;
	}
	return rtnValue;
}

