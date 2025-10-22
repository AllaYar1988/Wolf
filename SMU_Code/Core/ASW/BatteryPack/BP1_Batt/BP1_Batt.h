/*
 * S4_Batt.h
 *
 *  Created on: Jan 19, 2025
 *      Author: 98912
 */

#ifndef _BP1_BATT_H_
#define _BP1_BATT_H_

#define BP1_BATT_ADDRESS 0x7E
typedef struct Battery_HandleTypeDef {
	int Ver;
	int add;
	float Vtotal;
	float CurrentTotal;
	int crc;
	float Temp1;
	float Temp2;
	float Temp3;
	float PDC;
	float PFCC;
	int UDN;
	float PRC;
	int NCDC;
	float Temp4;
	float Tref;
	float Vblock;
	int Tblock;
	int tcount;

} Battery_HandleTypeDef;


extern Battery_HandleTypeDef BatteryPack1;
extern int chCom ;


void BP1_Batt_resProcess(char *res);
void BP1_BattSend(void);



#endif /* ASW_S4_BATT_S4_BATT_H_ */
