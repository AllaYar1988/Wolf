/*
 * MBS.c
 *
 *  Created on: Apr 8, 2025
 *      Author: 98912
 */

#include "MBS.h"
#include "MAC.h"
#include "cmd.h"
#include "dbg.h"

#include "BP2_SuproEnergy.h"
#include "BP3_Cyclenpo.h"
#include "BP1_Batt.h"
#include "BP_Mng.h"

//#include "../../../../ASW/BatteryPack/BP3_Cyclenpo/BP3_Cyclenpo.h"

u8 MBS_ParsData(void) ;



void MBS_Handler(void)
{
	static int state=0;
	BP_Mng();
	MAC_MbsSendData();

	switch(state)
	{
	case 0:
		if(3==MAC_MbsReciveData())
		{
			state=1;
		}
		break;
	case 1:
		if(0==MBS_ParsData())
		{
			state=0;
		}
		break;
	default:
		break;
	}

}



u8 MBS_ParsData(void) {
	MBTypeDef *mbs=getMbs();
	u8 returnVlaue=0;

     if (mbs->pData[0] == BP1_BATT_ADDRESS) {
		BP1_Batt_resProcess((char *)mbs->pData);
	  }
	  else if(mbs->pData[0]== BP2_SUPRO_ADDRESSS)
	  {
		BP2_SuproEnergyResProcess((char *)mbs->pData, mbs->byteCount);
	  }
	  else if((mbs->pData[0]== BP3_CYCLENPO_ADDRESSS) && (mbs->pData[1]== BP3_REALTIME_ADDRESSS))
	   {
		  Cyclenpo_realtime_Process((char *)mbs->pData, mbs->byteCount);
		  Process_complete=1;
	    }
	  else if((mbs->pData[0]== 0x12) && (mbs->pData[1]== BP3_REALTIME_ADDRESSS))
		   {
			  Cyclenpo_realtime_Process((char *)mbs->pData, mbs->byteCount);
			  Process_complete=1;
		    }
	  else if((mbs->pData[0]== BP3_CYCLENPO_ADDRESSS) && (mbs->pData[1]== BP3_PROTECTION_ADDRESSS))
	  	   {
		  Cyclenpo_batprtctn_Process((char *)mbs->pData, mbs->byteCount);
		  Process_complete=1;
		  send_ready=1;
	  	    }
	  else{
		  returnVlaue=executeCommand((u8 *)mbs->pData);
	  }
	 if(0==returnVlaue)memset(mbs->pData, 0, MBS_DMA_BUF_SIZE);
	 return returnVlaue;
}
//==============================
