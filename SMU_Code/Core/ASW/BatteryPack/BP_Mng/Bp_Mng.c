/*
 * Bp_Mng.c
 *
 *  Created on: Feb 11, 2025
 *      Author: 98912
 */

#include "BP_Mng.h"

#include <string.h>
#include "dbg.h"

static int bpSelect=BP3;

u8 BP_MngCommunication(char *str) {
	if (strstr((char*) str, "off")) {
		bpSelect = BPOFF;
		TransmitCMDResponse("Charger Communication turned Off \n\n");
	} else if (strstr((char*) str, "BP1")) {
		bpSelect = BP1;
		TransmitCMDResponse( "BP1 is ON now \n\n");
	}
	else if (strstr((char*) str, "BP2")) {
		bpSelect = BP2;
		TransmitCMDResponse( "BP2 is ON now \n\n");
	}
return 0;
}
u8 BP_MngCommunicationHelp(void)
{
	return 0;
}

int BP_Mng(void)
{
	int status=0;
	bpSelect = BP3;
	switch(bpSelect){
	case BPOFF:
		break;
	case BP1:
		BP1_BattSend();
		break;
	case BP2:
		status=BP2_SuperoEnergySend();
		break;
	case BP3:
		status=BP3_cyclenpobat_Send();
		break;
	}
	return status;
}
