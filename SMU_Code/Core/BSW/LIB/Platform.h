/*
 * Platform.h
 *
 *  Created on: Feb 24, 2025
 *      Author: 98912
 */

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define  u8 uint8_t
#define  u16 uint16_t
#define  u32 uint32_t
#define  f32 float

#define SW_VERSION "SWSYS Version: V3.26"
#define INVALID_DATA 3141.0

/*
 * Define the function variable with void input and output arguments.
 */
typedef void (*PVoidCallBackFuncType) (void);

#define NAME_SIZE 20
#define UNIT_SIZE 5

typedef struct {
	char name[NAME_SIZE];
	char unit[UNIT_SIZE];
	float value;

}MntDataType;





#endif /* BSW_LIB_PLATFORM_H_ */
