                                    /*******************
 ***************************************** C SOURCE FILE ******************************************
                                        *******************/
/**
 *  @file       MAC.H
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
#ifndef SRC_MAC_H_
#define SRC_MAC_H_
#include "stm32f4xx_hal.h"
#include <stdio.h>      // For sprintf, printf, etc.
#include <string.h>     // For strlen, memset
#include <stdlib.h>     // For atoi
#include <stdint.h>     // For uint8_t, etc.
#include "main.h"
#include "usart.h"

#define MbsUart huart6
#define MBS_DMA_BUF_SIZE 256
#define MBS_DMA_SEND_BUF_SIZE 500

#define MbmUart huart3
#define MBM_DMA_BUF_SIZE 256
#define MBM_DMA_SEND_BUF_SIZE 500

typedef struct
{
	uint8_t semaphore;
    uint8_t byteCount;
    uint8_t dstAddress;
    uint16_t dataLen;
    uint8_t sData[MBS_DMA_SEND_BUF_SIZE];
    uint8_t rData[MBS_DMA_BUF_SIZE];
    uint8_t pData[MBS_DMA_BUF_SIZE];
} MBTypeDef;

#define __SIO_SLAVE(x)  HAL_GPIO_WritePin (Master_Select_GPIO_Port,Master_Select_Pin,x);
#define __SIO_Master(x)   HAL_GPIO_WritePin(Slave_Select_GPIO_Port, Slave_Select_Pin,x);




int MAC_MbmSendData(uint8_t *mbStr,int mbDataLen);
int MAC_MbmReciveData(void);
MBTypeDef* getMbm(void);

void MAC_MailToMbs(char *mbStr,int mbDataLen);
int MAC_MbsSendData(void);
int MAC_MbsReciveData(void);
MBTypeDef *getMbs(void);
#endif /* SRC_MAC_H_ */
