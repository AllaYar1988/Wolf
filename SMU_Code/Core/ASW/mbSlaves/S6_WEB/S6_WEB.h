/*
 * S6_WEB.h
 *
 *  Created on: Feb 21, 2025
 *      Author: 98912
 */

#ifndef ASW_MBSLAVES_S6_WEB_S6_WEB_H_
#define ASW_MBSLAVES_S6_WEB_S6_WEB_H_

#include "Platform.h"
#include "string.h"
#include "modbus.h"


typedef struct{
uint32_t readTimeout;
uint32_t writeTimeout;
}S6_WEB;


void S6_WEB_registerToMbNode(void);

#endif /* ASW_MBSLAVES_S6_WEB_S6_WEB_H_ */
