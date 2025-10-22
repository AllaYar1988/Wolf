/*
 * energy_meter_hal.h
 *
 *  Created on: Oct 16, 2025
 *      Author:  Allahyar Moazami
 */

#ifndef BSW_HAL_ENERGY_METER_HAL_ENERGY_METER_HAL_H_
#define BSW_HAL_ENERGY_METER_HAL_ENERGY_METER_HAL_H_
#include "stm32f4xx_hal.h"

#define uGenerator_STPM_SCS_Pin GPIO_PIN_1
#define uGenerator_STPM_SCS_GPIO_Port GPIOB
#define ENER_GEN_CS(x)

extern UART_HandleTypeDef huart4;


#define ENERGY_METER_UART huart4

void energy_meters_hal_init(void);

#endif /* BSW_HAL_ENERGY_METER_HAL_ENERGY_METER_HAL_H_ */
