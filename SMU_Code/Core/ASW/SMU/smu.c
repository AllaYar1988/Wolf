/*
* S2_CH.c
*
*  Created on: Jan 19, 2025
*      Author: 98912
*/
#include "smu.h"
#include "crc.h"
#include "mntdata.h"
#include "S1_INV.h"
#include "S2_CH.h"


MntDataType smuMntData[SMU_MNT_DATA_Size]=
 {
          {"Live","N",0.0},     //0
	  {"TempE","C",0.0},     //0
          
 };
void SMU_Database_Init(void)
{
    registerToDatabase("SMU",
                      smuMntData,
                      SMU_MNT_DATA_Size);
}

void SMU_Slaves_Database_Init(void)
{
  SMU_Database_Init();
  S1_INV_Database_Init();
  S2_CH_Database_Init();
  

}

