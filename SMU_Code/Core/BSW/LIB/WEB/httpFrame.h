/*
 * httpFrame.h
 *
 *  Created on: Jan 8, 2025
 *      Author: 98912
 */

#ifndef SRC_HTTPFRAME_H_
#define SRC_HTTPFRAME_H_
#include <stdio.h>      // For sprintf, printf, etc.
#include <string.h>     // For strlen, memset
#include <stdlib.h>     // For atoi
#include <stdint.h>     // For uint8_t, etc.
#include "myrtc.h"
#include "../../SVC/COM/MDM/MdmSrv.h"

extern DateTime urtc,mdt,dt,urtcd;


void startJsonFrame( char *str, DateTime tm);
int addDataToJsonFrame( char *str);

#endif /* SRC_HTTPFRAME_H_ */
