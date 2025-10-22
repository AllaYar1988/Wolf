/*
 * MdmDll.h
 *
 *  Created on: Apr 8, 2025
 *      Author: 98912
 */

#ifndef BSW_HAL_COMHW_MDMDLL_H_
#define BSW_HAL_COMHW_MDMDLL_H_
#include "usart.h"
#include <stdint.h>
#include <string.h>

#define MdmUart huart1
#define MDM_DMA_BUF_SIZE 500
#define MDM_DMA_SEND_BUF_SIZE 500

typedef struct
{
    uint8_t byteCount;
    uint16_t dataLen;
    uint8_t rData[MDM_DMA_BUF_SIZE];
    uint8_t pData[MDM_DMA_BUF_SIZE];
} MDMTypeDef;


void MDM_SendData(uint8_t *str);
int MAC_MdmReciveData(void);
MDMTypeDef* getMdm(void);

#endif /* BSW_HAL_COMHW_MDMDLL_H_ */
