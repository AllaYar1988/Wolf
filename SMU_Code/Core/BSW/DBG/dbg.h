/*
 * dbg.h
 *
 *  Created on: Jan 19, 2025
 *      Author: 98912
 */

#ifndef _DBG_H_
#define _DBG_H_
#include "main.h"
#include "gpio.h"
#include <stdint.h>

#define DB1_Toggle HAL_GPIO_TogglePin(GPIOB,DB1_Pin);
#define DB2_Toggle HAL_GPIO_TogglePin(GPIOB,DB2_Pin);

#define DB2_SET_PIN HAL_GPIO_WritePin(GPIOB,DB2_Pin,GPIO_PIN_SET);
#define DB2_RESET_PIN HAL_GPIO_WritePin(GPIOB,DB2_Pin,GPIO_PIN_RESET);


extern UART_HandleTypeDef huart3;

void TransmitDebug(char *str);
void TransmitCMDResponse(char *str);
void mcuRstMsg(void);


#endif /* BSW_DBG_DBG_H_ */
