/*
 * energy_meter_hal.c
 *
 *  Created on: Oct 16, 2025
 *      Author:  Allahyar Moazami
 */

#include "energy_meter_hal.h"
#include "stm32f4xx_hal.h"



UART_HandleTypeDef huart4;




static void energy_meters_hal_serial_Init(void);

void energy_meters_hal_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*Configure GPIO pins : uGenerator_STPM_SCS */
	  GPIO_InitStruct.Pin = uGenerator_STPM_SCS_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	  energy_meters_hal_serial_Init();

}

/**
  * @brief ENERGY_METER_UART Initialization Function
  * @param None
  * @retval None
  */
static void energy_meters_hal_serial_Init(void)
{

	ENERGY_METER_UART.Instance = UART4;
	ENERGY_METER_UART.Init.BaudRate = 9600;
	ENERGY_METER_UART.Init.WordLength = UART_WORDLENGTH_8B;
	ENERGY_METER_UART.Init.StopBits = UART_STOPBITS_1;
	ENERGY_METER_UART.Init.Parity = UART_PARITY_NONE;
	ENERGY_METER_UART.Init.Mode = UART_MODE_TX_RX;
	ENERGY_METER_UART.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	ENERGY_METER_UART.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&ENERGY_METER_UART) != HAL_OK)
  {
    /* Error_Handler(); */
  }


}
