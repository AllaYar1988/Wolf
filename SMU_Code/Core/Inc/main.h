/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define uRI_Pin GPIO_PIN_2
#define uRI_GPIO_Port GPIOE
#define uPWRKEY_Pin GPIO_PIN_3
#define uPWRKEY_GPIO_Port GPIOE
#define uReset_Pin GPIO_PIN_4
#define uReset_GPIO_Port GPIOE
#define uControl3V9_Pin GPIO_PIN_7
#define uControl3V9_GPIO_Port GPIOE
#define DS18B20_Pin GPIO_PIN_4
#define DS18B20_GPIO_Port GPIOA
#define DB1_Pin GPIO_PIN_0
#define DB1_GPIO_Port GPIOB
#define DB2_Pin GPIO_PIN_1
#define DB2_GPIO_Port GPIOB
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define WDI_Supervisor_Pin GPIO_PIN_3
#define WDI_Supervisor_GPIO_Port GPIOC
#define uRGB_B_Pin GPIO_PIN_8
#define uRGB_B_GPIO_Port GPIOE
#define uRGB_G_Pin GPIO_PIN_10
#define uRGB_G_GPIO_Port GPIOE
#define uRGB_R_Pin GPIO_PIN_12
#define uRGB_R_GPIO_Port GPIOE
#define Slave_Select_Pin GPIO_PIN_15
#define Slave_Select_GPIO_Port GPIOE
#define uSlave_TX_Pin GPIO_PIN_10
#define uSlave_TX_GPIO_Port GPIOB
#define uSlave_RX_Pin GPIO_PIN_11
#define uSlave_RX_GPIO_Port GPIOB
#define uRelayLoad_Pin GPIO_PIN_11
#define uRelayLoad_GPIO_Port GPIOD
#define uRelayGridN_Pin GPIO_PIN_12
#define uRelayGridN_GPIO_Port GPIOD
#define uRelayGridP_Pin GPIO_PIN_13
#define uRelayGridP_GPIO_Port GPIOD
#define uRelayGen_Pin GPIO_PIN_14
#define uRelayGen_GPIO_Port GPIOD
#define Master_Select_Pin GPIO_PIN_15
#define Master_Select_GPIO_Port GPIOD
#define uMaster_TX_Pin GPIO_PIN_6
#define uMaster_TX_GPIO_Port GPIOC
#define uMaster_RX_Pin GPIO_PIN_7
#define uMaster_RX_GPIO_Port GPIOC
#define uGPRS_TX_Pin GPIO_PIN_9
#define uGPRS_TX_GPIO_Port GPIOA
#define uGPRS_RX_Pin GPIO_PIN_10
#define uGPRS_RX_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_12
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_15
#define LED2_GPIO_Port GPIOA
#define uDi3_Pin GPIO_PIN_1
#define uDi3_GPIO_Port GPIOD
#define SD_Detect_Pin GPIO_PIN_3
#define SD_Detect_GPIO_Port GPIOD
#define uDi2_Pin GPIO_PIN_4
#define uDi2_GPIO_Port GPIOD
#define uDi1_Pin GPIO_PIN_5
#define uDi1_GPIO_Port GPIOD
#define uRelay5_Pin GPIO_PIN_7
#define uRelay5_GPIO_Port GPIOD
#define uRelay4_Pin GPIO_PIN_3
#define uRelay4_GPIO_Port GPIOB
#define uRelay3_Pin GPIO_PIN_4
#define uRelay3_GPIO_Port GPIOB
#define uRelay2_Pin GPIO_PIN_5
#define uRelay2_GPIO_Port GPIOB
#define uSCL_Pin GPIO_PIN_6
#define uSCL_GPIO_Port GPIOB
#define uSDA_Pin GPIO_PIN_7
#define uSDA_GPIO_Port GPIOB
#define uRelay1_Pin GPIO_PIN_8
#define uRelay1_GPIO_Port GPIOB
#define uDo2_Pin GPIO_PIN_9
#define uDo2_GPIO_Port GPIOB
#define uCTSM_Pin GPIO_PIN_0
#define uCTSM_GPIO_Port GPIOE
#define uDo1_Pin GPIO_PIN_1
#define uDo1_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
