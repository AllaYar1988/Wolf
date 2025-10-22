/*
 * dbg.c
 *
 *  Created on: Jan 19, 2025
 *      Author: 98912
 */
#include "dbg.h"
#include "sysvar.h"
#include "modbus.h"
#include <string.h>
#include <stdio.h>
#include "MAC.h"

void mcuRstMsg(void)
{
	TransmitDebug("\r************************************\r");
	TransmitCMDResponse("\rProgram Start ...\r");
	TransmitDebug("\r************************************\r");
}


void TransmitDebug(char *str) {
	if (0 != dbgCtrl)
	{
	MAC_MailToMbs(str,strlen(str));

	}
}

void TransmitCMDResponse(char *str) {

	MAC_MailToMbs(str,strlen(str));

}
