/*
 * BP2_SuproEnergy.c
 *
 *  Created on: Feb 11, 2025
 *      Author: 98912
 */

#include "BP2_SuproEnergy.h"
#include "crc.h"
#include "MAC.h"

#define size 10
static char bString[size];
int S3_HMI_sendWriteReq(void);

Supro_HandleTypeDef SuproEnergy1 = { .Slave_add = 0x01 };


int BP2_To_HMI_sendWriteReq(void);


void BP2_SuproEnergyInit(void){
	bString[0]=0x01;
	bString[1]=0x78;
	bString[2]=0x10;
	bString[3]=0x00;
	bString[4]=0x10;
	bString[5]=0xa0;
	bString[6]=0x00;
	bString[7]=0x00;
	bString[8]=0x7f;
	bString[9]=0xb2;
	/*	bString[10]=0xFF;
	bString[11]=0xFF;
	bString[12]=0xFF;
	bString[13]=0xFF;
	bString[14]=0xFF;
	bString[15]=0xFF;
	bString[16]=0xFF;
	bString[17]=0xFF;
	bString[18]=0xFF;
	bString[19]=0xFF;*/
}

void BP2_SuproEnergyResProcess(char *str,int Len)
{
	if(checkFrameCRC(str,Len))
	{
		switch(str[1])
		{
		case 0x78:
		{
			SuproEnergy1.Vtotal = (str[8]<<8 |str[9])*0.01;
			SuproEnergy1.V_busbar= (str[10]<<8 | str[11])*0.01;
			SuproEnergy1.CurrentTotal= (str[12]<<24 | str[13]<<16 |str[14]<<8 |str[15])*0.01-3000;
			SuproEnergy1.SOC= (str[16]<<8 | str[17])*0.01;
			SuproEnergy1.Remaining_Cap= (str[18]<<8 | str[19])*0.01;
			SuproEnergy1.Full_cap= (str[20]<<8 | str[21])*0.01;
			SuproEnergy1.Nom_cap= (str[22]<<8 | str[23])*0.01;
			SuproEnergy1.T_MOS= (str[24]<<8 | str[25])*0.1-50;
			SuproEnergy1.T_amb= (str[26]<<8 | str[27])*0.1-50;
			SuproEnergy1.Status= (str[28]<<8 | str[29]);
			SuproEnergy1.SOH= (str[30]<<8 | str[31]);
			SuproEnergy1.Protection_code= (str[32]<<24 | str[33]<<16 |str[34]<<8 | str[35]);
			SuproEnergy1.Warning_code= (str[36]<<24 | str[37]<<16 |str[38]<<8 | str[39]);
			SuproEnergy1.MOS_state= (str[40]<<8 |str[41]);
			SuproEnergy1.Signal_status= (str[42]<<8 | str[43]);
			SuproEnergy1.Cycle_times= (str[44]<<8 | str[45]);
			SuproEnergy1.Max_mono= (str[46]<<8 | str[47]);
			SuproEnergy1.Max_V_cell= (str[48]<<8 |str[49]);
			SuproEnergy1.Min_mono= (str[50]<<8 | str[51]);
			SuproEnergy1.Min_V_cell= (str[52]<<8 | str[53]);
			SuproEnergy1.V_ave= (str[54]<<8 | str[55]);
			SuproEnergy1.T_max_SN= (str[56]<<8 | str[57]);
			SuproEnergy1.T_max= (str[58]<<8 | str[59])*0.1-50;
			SuproEnergy1.T_min_SN= (str[60]<<8 |str[61]);
			SuproEnergy1.T_min= (str[62]<<8 | str[63])*0.1-50;
			SuproEnergy1.T_ave= (str[64]<<8 | str[65])*0.1-50;
			SuproEnergy1.V_max_charge= (str[66]<<8 | str[67])*0.1;
			SuproEnergy1.I_max_charge= (str[68]<<8 | str[69])*0.1;
			SuproEnergy1.V_max_discharge= (str[70]<<8 | str[71])*0.1;
			SuproEnergy1.I_max_discharge= (str[72]<<8 | str[73])*0.1;
			SuproEnergy1.Num_Mono_N= (str[74]<<8 | str[75]);
			int data_ctr = 0;
			for (data_ctr = 0; data_ctr < SuproEnergy1.Num_Mono_N; data_ctr++)
			{
				SuproEnergy1.V_cell[data_ctr] = (str[76+2*data_ctr]<<8 | str[77+2*data_ctr]);
			}
			SuproEnergy1.Num_Temp = (str[76+2*data_ctr]<<8 |str[77+2*data_ctr]);
			data_ctr++;
			for (int temp_ctr = 0; temp_ctr < SuproEnergy1.Num_Temp; temp_ctr++)
			{
				SuproEnergy1.T_cell[temp_ctr] = (str[76+2*data_ctr]<<8 | str[77+2*data_ctr])*0.1-50;
				data_ctr++;
			}

			_memoryMap[238] = (int) (SuproEnergy1.Vtotal * 100); //(BatteryPack1.Vtotal * 100);
			_memoryMap[239] = (int) (SuproEnergy1.SOC * 100); //(BatteryPack1.CurrentTotal * 100);
			_memoryMap[240] = (int) (SuproEnergy1.T_amb * 100); //(BatteryPack1.Temp1 * 100);

		}
		break;
		default:
			break;
		}
	}
}



int BP2_SuperoEnergySend(void){
	//7E 32 35 30 32 34 36 34 32 45 30 30 32 30 32 46 44 32 45 0D
	static uint32_t supTick=0;
	static int state=0;
	int status=0;
	switch(state)
	{
	case 0:
		BP2_SuproEnergyInit();
		state=1;
		break;
	case 1:
		if(HAL_GetTick()-supTick>15000)
	 {
			status=1;

			MAC_MailToMbs(bString,10);

				state=2;

		}
		break;
	case 2:
		status=1;
		if(0==BP2_To_HMI_sendWriteReq()){
			status=0;
			supTick=HAL_GetTick();
			state=1;
		}
		break;
	}
	return status;
}

int BP2_To_HMI_sendWriteReq(void){
	static u8 mbStr[256];
	int status = 1;
	static u16 kindex = 7;
	static int state=0;
	int BP2ToHmimemoryMap[50]={0};
	switch(state)
	{
	case 0:
		BP2ToHmimemoryMap[0] =  (int) (SuproEnergy1.Vtotal * 10);
		BP2ToHmimemoryMap[1] =  (int) (SuproEnergy1.CurrentTotal * 10);
		BP2ToHmimemoryMap[2] =  (int) (SuproEnergy1.SOC * 10);
		BP2ToHmimemoryMap[3] =  (int) (SuproEnergy1.Remaining_Cap * 10);
		BP2ToHmimemoryMap[4] =  (int) (SuproEnergy1.T_MOS * 10);
		BP2ToHmimemoryMap[5] =  (int) (SuproEnergy1.T_amb * 10);
		BP2ToHmimemoryMap[6] =  (int) (SuproEnergy1.Status * 10);
		BP2ToHmimemoryMap[7] =  (int) (SuproEnergy1.Protection_code * 10);
		BP2ToHmimemoryMap[8] =  (int) (SuproEnergy1.Warning_code * 10);
		BP2ToHmimemoryMap[9] =  (int) (SuproEnergy1.T_max * 10);
		BP2ToHmimemoryMap[10] = (int) (SuproEnergy1.T_min * 10);
		BP2ToHmimemoryMap[11] = (int) (SuproEnergy1.T_cell[0] * 10);
		BP2ToHmimemoryMap[12] = (int) (SuproEnergy1.T_cell[1] * 10);
		BP2ToHmimemoryMap[13] = (int) (SuproEnergy1.T_cell[2] * 10);
		BP2ToHmimemoryMap[14] = (int) (SuproEnergy1.T_cell[3] * 10);
		mbStr[0]= 0x02;
		mbStr[1] =  0x10;
		mbStr[2] = (BP2ToHmi_WRITE_START_ADDRESS & 0xff00) >> 8;
		mbStr[3] =  BP2ToHmi_WRITE_START_ADDRESS & 0x00ff;
		mbStr[4] = (BP2ToHmi_WRITE_NUMBER_BYTE & 0xff00) >> 8;
		mbStr[5] =  BP2ToHmi_WRITE_NUMBER_BYTE & 0x00ff;
		mbStr[6] = 2 * BP2ToHmi_WRITE_NUMBER_BYTE;
		kindex = 7;
		for (int index = 0; index < BP2ToHmi_WRITE_NUMBER_BYTE; index++) {
			mbStr[kindex++] = (((int) (BP2ToHmimemoryMap[index])) & 0xff00) >> 8;
			mbStr[kindex++] = ((int)  (BP2ToHmimemoryMap[index])) & 0x00ff;

		}
		u16 crc = calculateCRC(mbStr, kindex); //
		mbStr[kindex++] = crc & 0x00ff;
		mbStr[kindex++] = (crc & 0xff00) >> 8;
		state=1;
		break;
	case 1:
		if(0==MAC_MbmSendData((uint8_t*)mbStr,kindex)){
			state=0;
			status=0;
			memset(mbStr,0,100);
		}
		break;
	}
	return status;

}
