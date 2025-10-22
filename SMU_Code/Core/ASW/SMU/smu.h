/*
 * S2_CH.h
 *
 *  Created on: Jan 19, 2025
 *      Author: 98912
 */

#ifndef _SMU_H_
#define _SMU_H_
#include "main.h"
#include "modbus.h"
#include "string.h"


//#define SMU_ID 0x00


typedef enum{
        SMU_MNT_LIVE=0,
	SMU_MNT_TEMPE,       /*0*/
        SMU_MNT_DATA_Size
}SMU_MntEnumType;


void SMU_Database_Init(void);
void SMU_Slaves_Database_Init(void);

#endif /* ASW_S2_CH_S2_CH_H_ */
