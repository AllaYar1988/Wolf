/*
 * S4_Batt.c
 *
 *  Created on: Jan 19, 2025
 *      Author: 98912
 */

#include "BP1_Batt.h"

#include "main.h"
#include "modbus.h"
#include "mntdata.h"
#include <string.h>
#include "MAC.h"

#define size 20
static  char _bstring[size];
Battery_HandleTypeDef BatteryPack1;
int chCom = 1;


static int htoa(char st, char shiftt);
static void BP1_BattInit(void);

void BP1_BattInit(void)
{
	_bstring[0] = 0x7E;
	_bstring[1] = 0x32;
	_bstring[2] = 0x35;
	_bstring[3] = 0x30;
	_bstring[4] = 0x32;
	_bstring[5] = 0x34;
	_bstring[6] = 0x36;
	_bstring[7] = 0x34;
	_bstring[8] = 0x32;
	_bstring[9] = 0x45;
	_bstring[10] = 0x30;
	_bstring[11] = 0x30;
	_bstring[12] = 0x32;
	_bstring[13] = 0x30;
	_bstring[14] = 0x32;
	_bstring[15] = 0x46;
	_bstring[16] = 0x44;
	_bstring[17] = 0x32;
	_bstring[18] = 0x45;
	_bstring[19] = 0x0D;

	BatteryPack1.Tblock = 2;
}
void BP1_Batt_resProcess(char *res)
{
	int dummy22 = 0;

	if (res[143] == 0x0D) {
		BatteryPack1.Ver = (res[1]) - 48;
		BatteryPack1.Ver = (int) (BatteryPack1.Ver * 10
				+ (res[2]) - 48);
		BatteryPack1.add = (res[3] - 48) * 10
				+ (res[4] - 48);

		if (BatteryPack1.add == 2) {
			dummy22 = htoa(res[107], 12)
								+ htoa(res[108], 8)
								+ htoa(res[109], 4)
								+ htoa(res[110], 0);
			BatteryPack1.CurrentTotal = (float) dummy22 / 10000;

			dummy22 = htoa(res[111], 12)
								+ htoa(res[112], 8)
								+ htoa(res[113], 4)
								+ htoa(res[114], 0);

			BatteryPack1.Vtotal = (float) dummy22 / 100;
			dummy22 = htoa(res[103], 12)
								+ htoa(res[104], 8)
								+ htoa(res[105], 4)
								+ htoa(res[106], 0);
			BatteryPack1.Temp1 = (float) dummy22 - 40;

			BatteryPack1.PDC = (res[131] - 48) * 1000
					+ (res[132] - 48) * 100
					+ (res[133] - 48) * 10
					+ (res[134] - 48);
			BatteryPack1.PDC = (float) BatteryPack1.PDC / 100;

		}

	}
}

int htoa(char st, char shiftt) {
	int r = 0;
	char stt = 0;
	stt = st - 0x30;

	switch (stt) {
	case 0:
		r = 0;
		break;
	case 1:
		r = 1;
		break;
	case 2:
		r = 2;
		break;
	case 3:
		r = 3;
		break;
	case 4:
		r = 4;
		break;
	case 5:
		r = 5;
		break;
	case 6:
		r = 6;
		break;
	case 7:
		r = 7;
		break;
	case 8:
		r = 8;
		break;
	case 9:
		r = 9;
		break;
	case 0x11:
		r = 10;
		break;
	case 0x12:
		r = 11;
		break;
	case 0x13:
		r = 12;
		break;
	case 0x14:
		r = 13;
		break;
	case 0x15:
		r = 14;
		break;
	case 0x16:
		r = 15;
		break;
	default:
		r = 0;
		break;

	}
	r = r << shiftt;
	return r;

}




void BP1_BattSend(void){
	//7E 32 35 30 32 34 36 34 32 45 30 30 32 30 32 46 44 32 45 0D
	static uint32_t battTick=0;
	static int state=0;
	switch(state)
	{
	case 0:
		BP1_BattInit();
		state=1;
		break;
	case 1:
		if(HAL_GetTick()-battTick>5000)
		{
			MAC_MailToMbs(_bstring,20);
			battTick=HAL_GetTick();
		}
		break;

	}

}
