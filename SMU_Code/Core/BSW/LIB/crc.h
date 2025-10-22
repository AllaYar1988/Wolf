/*
 * crc.h
 *
 *  Created on: Jan 19, 2025
 *      Author: 98912
 */

#ifndef BSW_LIB_CRC_H_
#define BSW_LIB_CRC_H_
#include <stdbool.h>
#include "Platform.h"

u16 calculateCRC(u8 *frame, u16 len) ;
bool checkFrameCRC(char frame[], int frameSize) ;

// Calculate simple CRC8

u8 crc_stpm3x(u8 *data, u8 len);

#endif /* BSW_LIB_CRC_H_ */
