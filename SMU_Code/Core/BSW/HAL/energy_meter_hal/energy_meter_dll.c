/*
 * energy_meters_dll.c
 *
 *  Created on: Oct 16, 2025
 *      Author: Allahyar Moazami
 */


#include "energy_meter_dll.h"
#include "energy_meter_hal.h"
#include "dbg.h"
#include "platform.h"
#include "usart.h"
#include "stm32f4xx_hal_uart.h"


#define ENERGY_BUFFER_SIZE 100


u8 gEnergyData[ENERGY_BUFFER_SIZE];

/*!
 **************************************************************************************************
 *
 *  @fn          void energy_meter_dll_recive (void)
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
void energy_meter_dll_send(u8 *msg, u16 size)
{

	ENERGY_METER_UART.gState = HAL_UART_STATE_READY;
	HAL_UART_Transmit_DMA(&ENERGY_METER_UART,msg,size);
}



/*!
 **************************************************************************************************
 *
 *  @fn          u16 energy_meter_dll_recive (void)
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
u16 energy_meter_dll_recive(void)
{
	static int state=0;
	int rtnValue=0;
	switch(state)
	{
	case 0:
		HAL_UART_Receive_DMA(&ENERGY_METER_UART, (uint8_t*) gEnergyData, ENERGY_BUFFER_SIZE);
		state=1;
		break;
	case 1:
		if (0)//__HAL_UART_GET_FLAG(&ENERGY_METER_UART, UART_FLAG_IDLE))
		{

			HAL_GPIO_WritePin(uCTSM_GPIO_Port,uCTSM_Pin,GPIO_PIN_SET);

			uint32_t tmp;
			// Clear the idle flag by reading the status register and then the data register.
			// This prevents the flag from retriggering immediately.
			//HAL_UART_DMAStop(&ENERGY_METER_UART);
			//tmp = ENERGY_METER_UART.Instance->SR;
			//tmp = ENERGY_METER_UART.Instance->DR;
			(void)tmp; // Avoid compiler warning about unused variable
			//memcpy(mdm.pData,mdm.rData,MDM_DMA_BUF_SIZE);
			//memset(mdm.rData,0,MDM_DMA_BUF_SIZE);
			//mdm.byteCount= MDM_DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(MdmUart.hdmarx);
			rtnValue=3;
			//HAL_UART_Receive_DMA(&ENERGY_METER_UART, (uint8_t*) mdm.rData, MDM_DMA_BUF_SIZE); //GSM
		}
		break;
	default:
		break;
	}
	return rtnValue;
}

