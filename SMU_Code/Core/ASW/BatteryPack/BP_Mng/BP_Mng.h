/*
 * BP_Mng.h
 *
 *  Created on: Feb 11, 2025
 *      Author: 98912
 */

#ifndef ASW_BATTERYPACK_BP_MNG_BP_MNG_H_
#define ASW_BATTERYPACK_BP_MNG_BP_MNG_H_


#include "BP1_Batt.h"
#include "BP2_SuproEnergy.h"
#include "Platform.h"

#define BPOFF 0
#define BP1 1
#define BP2 2
#define BP3 3

int BP_Mng(void);

u8 BP_MngCommunication(char *str) ;
u8 BP_MngCommunicationHelp(void) ;

#endif /* ASW_BATTERYPACK_BP_MNG_BP_MNG_H_ */
