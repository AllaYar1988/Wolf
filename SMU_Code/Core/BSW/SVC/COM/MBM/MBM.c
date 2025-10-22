/*
 * MBM.c
 *
 *  Created on: Apr 8, 2025
 *      Author: 98912
 */


#include "MBM.h"
#include "MAC.h"
#include "RTE_MB.h"

void MBM_ParsData() ;



void MBM_Handler(void)
{
	static int state=0;
	RTE_MB_Mng();
	switch(state)
	{
	case 0:
		if(3==MAC_MbmReciveData())
		{
			state=1;
		}
		break;
	case 1:
		MBM_ParsData();
		state=0;
		break;
	default:
		break;
	}

}


void MBM_ParsData() {
	MBTypeDef *mbm=getMbm();

	RTE_MB_Rec_Mng(mbm->pData,mbm->byteCount);
	 memset(mbm->pData, 0, MBM_DMA_BUF_SIZE);
}
