/*
 * BP2_SuproEnergy.c
 *
 *  Created on: Feb 11, 2025
 *      Author: 98912
 */

#include "../BP3_Cyclenpo/BP3_Cyclenpo.h"

#include "crc.h"
#include "MAC.h"
#include "mntdata.h"

#define size 10

static char RString[size];
static char PString[size];
int S3_HMI_sendWriteReq(void);

Cyclenpo_HandleTypeDef Cyclenpo = { .Slave_add = 0x02 };
int Process_complete=1;
int send_ready=0;

MntDataType mntBData[20]=
{
		{"Vtotal1","V",0.0},//1
		{"Itotal1","A",0.0},//2
		{"SOC1","N",0.0},//3
		{"SOH1","N",0.0},//4
		{"MaxVcell1","V",0.0},//5
		{"Tmax1","C",0.0},//6
		{"Tmos1","C",0.0},//7
		{"PrtCode1","A",0.0},//8
		{"WarCode1","A",0.0},//9
		{"Vtotal2","V",0.0},//1
		{"Itotal2","A",0.0},//2
		{"SOC2","N",0.0},//3
		{"SOH2","N",0.0},//4
		{"MaxVcell2","V",0.0},//5
		{"Tmax2","C",0.0},//6
		{"Tmos2","C",0.0},//7
		{"PrtCode2","A",0.0},//8
		{"WarCode2","A",0.0}//9
};


int BP3_To_HMI_sendWriteReq(void);


void BP3_cyclenpo_realtime_read(void)
{

		registerToDatabase("Batt12",
				mntBData,
				18);


	RString[0]=0x11;
	RString[1]=0x04;
	RString[2]=0x10;
	RString[3]=0x00;
	RString[4]=0x00;
	RString[5]=0x57;
	RString[6]=0xb7;
	RString[7]=0xa4;
}

void BP3_cyclenpo_batprtctn_read(void)
{
	PString[0]=0x11;
	PString[1]= 0x03;
	PString[2]=0x18;
	PString[3]=0x00;
	PString[4]=0x00;
	PString[5]=0x65;
	PString[6]=0x81;
	PString[7]=0xD1;

//	PString[0]=0x11;
//		PString[1]= 0x03;
//		PString[2]=0x18;
//		PString[3]=0x00;
//		PString[4]=0x00;
//		PString[5]=0x66;
//		PString[6]=0xE6;
//		PString[7]=0x84;
}
void Cyclenpo_realtime_Process(char *str,int Len)
{

	int i=0;
	if(checkFrameCRC(str,Len))
	{
		switch(str[1])
		{
		case 0x04:
		{
			Database_Type* mDb=getMntDatabase();
				MntDataType* mData = mDb[3].mData;


			Cyclenpo.Vtotal           = ((str[3]  << 8) | str[4])*0.01;			//Volt
			Cyclenpo.CurrentTotal     = ((str[5]  << 8) | str[6])*0.01;			//A
			Cyclenpo.SOC              = (str[7]  << 8) | str[8];				//%
			Cyclenpo.SOH              = (str[9]  << 8) | str[10];
			Cyclenpo.Max_mono         = (str[11] << 8) | str[12];				//1 to 32
			Cyclenpo.Max_V_cell       = (str[13] << 8) | str[14];				//mV
			Cyclenpo.Min_mono         = (str[15] << 8) | str[16];
			Cyclenpo.Min_V_cell       = (str[17] << 8) | str[18];				//mV
			Cyclenpo.T_max_SN         = (str[19] << 8) | str[20];
			Cyclenpo.T_max            = (((str[21] << 8) | str[22])*0.1)-50;	//Degree C
			Cyclenpo.T_min_SN         = (str[23] << 8) | str[24];				//1 to 16
			Cyclenpo.T_min            = (((str[25] << 8) | str[26])*0.1)-50;	//Degree C
			Cyclenpo.T_amb            = (((str[27] << 8) | str[28])*0.1)-50;	//Degree C
			Cyclenpo.T_MOS            = (((str[29] << 8) | str[30])*0.1)-50;	//Degree C
			Cyclenpo.Status           = (str[31] << 8) | str[32];				//read value
			Cyclenpo.Full_cap         = (((str[33] << 8) | str[34])*0.01);		//Ah
			Cyclenpo.Surplus_cap      = (((str[35] << 8) | str[36])*0.01);		//Ah
			Cyclenpo.Protection_code  = (str[37] << 24|str[38]<<16|str[39]<<8|str[40]);
			Cyclenpo.Warning_code     = (str[41]<< 24| str[42]<<16| str[43] <<8 | str[44]);

			// SN_Code is length 5 now:
			Cyclenpo.SN_Code[0] = str[45] ;
			Cyclenpo.SN_Code[1] = str[46] ;
			Cyclenpo.SN_Code[2] = str[47] ;
			Cyclenpo.SN_Code[3] = str[48] ;
			Cyclenpo.SN_Code[4] = str[49] ;
			Cyclenpo.SN_Code[5] = str[50] ;
			Cyclenpo.SN_Code[6] = str[51] ;
			Cyclenpo.SN_Code[7] = str[52] ;
			Cyclenpo.SN_Code[8] = str[53] ;
			Cyclenpo.SN_Code[9] = str[54] ;
			Cyclenpo.SN_Code[10] = str[55] ;
			Cyclenpo.SN_Code[11] = str[56] ;
			Cyclenpo.SN_Code[12] = str[57] ;
			Cyclenpo.SN_Code[13] = str[58] ;
			Cyclenpo.SN_Code[14] = str[59] ;
			Cyclenpo.SN_Code[15] = str[60] ;
			Cyclenpo.SN_Code[16] = str[61] ;
			Cyclenpo.SN_Code[17] = str[62] ;
			Cyclenpo.SN_Code[18] = str[63] ;
			Cyclenpo.SN_Code[19] = str[64] ;

			Cyclenpo.V_max_charge     = ((str[65] << 8) | str[66])*0.1;			//V
			Cyclenpo.I_max_charge     = ((str[67] << 8) | str[68])*0.1;			//A
			Cyclenpo.V_max_discharge  = ((str[69] << 8) | str[70])*0.1;			//V
			Cyclenpo.I_max_discharge  = ((str[71] << 8) | str[72])*0.1;			//A
			Cyclenpo.fill_time        = (str[73] << 8) | str[74];				//min
			Cyclenpo.time_of_release  = (str[75] << 8) | str[76];				//min
			Cyclenpo.parallel_num     = (str[77] << 8) | str[78];
			Cyclenpo.On_line_parallel = (str[79] << 8) | str[80];
			Cyclenpo.Cycle_times      = (str[81] << 8) | str[82];
			Cyclenpo.Running_status   = (str[83] << 8) | str[84];
			Cyclenpo.Num_Mono_N       = (str[85] << 8) | str[86];
			for(i=0;i<Cyclenpo.Num_Mono_N;i++)
			{
			Cyclenpo.V_cell[i] = str[87+(i*2)]<<8|str[88+(i*2)];				//mV
			}
			Cyclenpo.Num_Temp = (str[119] << 8)|str[120];
			for(i=0;i<Cyclenpo.Num_Temp;i++)
			{
			Cyclenpo.T_cell[i]=((str[121+(i*2)]<<8|str[122+(i*2)])*0.1)-50;		//Degree C
			}
			if(str[0]==0x11)
			{
			mData[0     ].value  = Cyclenpo.Vtotal;
					mData[1   ].value  = Cyclenpo.CurrentTotal;
					mData[2   ].value  = Cyclenpo.SOC;
					mData[3     ].value  = Cyclenpo.SOH;
					mData[4     ].value  = Cyclenpo.Max_V_cell;
					mData[5     ].value  = Cyclenpo.T_max;
					mData[6     ].value  = Cyclenpo.T_MOS;
					mData[7    ].value  = Cyclenpo.Protection_code;
					mData[8    ].value  = Cyclenpo.Warning_code;
			}
			if(str[0]==0x12)
				{
				mData[9     ].value  = Cyclenpo.Vtotal;
						mData[10   ].value  = Cyclenpo.CurrentTotal;
						mData[11   ].value  = Cyclenpo.SOC;
						mData[12    ].value  = Cyclenpo.SOH;
						mData[13     ].value  = Cyclenpo.Max_V_cell;
						mData[14     ].value  = Cyclenpo.T_max;
						mData[15     ].value  = Cyclenpo.T_MOS;
						mData[16    ].value  = Cyclenpo.Protection_code;
						mData[17   ].value  = Cyclenpo.Warning_code;
				}

		}
		break;
		default:
			break;
		}
	}
}

void Cyclenpo_batprtctn_Process(char *str,int Len)
{

		if(checkFrameCRC(str,Len))
		{
			switch(str[1])
			{
	case 0x03:
	{
		Cyclenpo.OV_cell_alrm = (str[3] << 8) | str[4];						//mV
		Cyclenpo.OV_cell_alrm_rels = (str[5] << 8) | str[6];				//mV
		Cyclenpo.OV_cell_alrm_delay = (str[7] << 8) | str[8];				//mS
		Cyclenpo.OV_cell_prtctn = (str[9] << 8) | str[10];					//mV
		Cyclenpo.OV_cell_prtctn_rels = (str[11] << 8) | str[12];			//mV
		Cyclenpo.OV_cell_prtctn_delay = (str[13] << 8) | str[14];			//mS

		Cyclenpo.UV_cell_alrm = (str[15] << 8) | str[16];					//mV
		Cyclenpo.UV_cell_alrm_rels = (str[17] << 8) | str[18];				//mV
		Cyclenpo.UV_cell_alrm_delay = (str[19] << 8) | str[20];				//mS
		Cyclenpo.UV_cell_prtctn = (str[21] << 8) | str[22];					//mV
		Cyclenpo.UV_cell_prtctn_rels = (str[23] << 8) | str[24];			//mV
		Cyclenpo.UV_cell_prtctn_delay = (str[25] << 8) | str[26];			//mS

		Cyclenpo.OV_pack_alrm = ((str[27] << 8) | str[28])*0.01;			//V
		Cyclenpo.OV_pack_alrm_rels = ((str[29] << 8) | str[30])*0.01;		//V
		Cyclenpo.OV_pack_alrm_delay = (str[31] << 8) | str[32];				//mS
		Cyclenpo.OV_pack_prtctn = ((str[33] << 8) | str[34])*0.01;			//V
		Cyclenpo.OV_pack_prtctn_rels = ((str[35] << 8) | str[36])*0.01;		//V
		Cyclenpo.OV_pack_prtctn_delay = (str[37] << 8) | str[38];			//mS

		Cyclenpo.UV_pack_alrm = ((str[39] << 8) | str[40])*0.01;			//V
		Cyclenpo.UV_pack_alrm_rels = ((str[41] << 8) | str[42])*0.01;		//V
		Cyclenpo.UV_pack_alrm_delay = (str[43] << 8) | str[44];				//mS
		Cyclenpo.UV_pack_prtctn = ((str[45] << 8) | str[46])*0.01;			//V
		Cyclenpo.UV_pack_prtctn_rels = ((str[47] << 8) | str[48])*0.01;		//V
		Cyclenpo.UV_pack_prtctn_delay = (str[49] << 8) | str[50];			//mS

		Cyclenpo.charge_OI_alrm = ((str[51] << 8) | str[52])*0.1;			//A
		Cyclenpo.charge_OI_alrm_rels = ((str[53] << 8) | str[54])*0.1;		//A
		Cyclenpo.charge_OI_alrm_delay = (str[55] << 8) | str[56];			//mS
		Cyclenpo.charge_OI_prtctn = ((str[57] << 8) | str[58])*0.1;			//A
		Cyclenpo.charge_OI_prtctn_delay = (str[59] << 8) | str[60];			//mS
		Cyclenpo.charge_OI_rels_delay = (str[61] << 8) | str[62];			//S
		Cyclenpo.charge_OI_lock_times = (str[63] << 8) | str[64];
		Cyclenpo.charge2_OI_prtctn = ((str[65] << 8) | str[66])*0.1;		//A
		Cyclenpo.charge2_OI_prtctn_delay = (str[67] << 8) | str[68];		//mS
		Cyclenpo.charge2_OI_rels_delay = (str[69] << 8) | str[70];			//S

		Cyclenpo.discharge_OI_alrm = ((str[71] << 8) | str[72])*0.1;		//A
		Cyclenpo.discharge_OI_alrm_rels = ((str[73] << 8) | str[74])*0.1;	//A
		Cyclenpo.discharge_OI_alrm_delay = (str[75] << 8) | str[76];		//mS
		Cyclenpo.discharge_OI_prtctn = ((str[77] << 8) | str[78])*0.1;		//A
		Cyclenpo.discharge_OI_prtctn_delay = (str[79] << 8) | str[80];		//mS
		Cyclenpo.discharge_OI_rels_delay = (str[81] << 8) | str[82];		//S
		Cyclenpo.discharge_OI_lock_times = (str[83] << 8) | str[84];
		Cyclenpo.discharge2_OI_prtctn = ((str[85] << 8) | str[86])*0.1;		//A
		Cyclenpo.discharge2_OI_prtctn_delay = (str[87] << 8) | str[88];		//mS
		Cyclenpo.discharge2_OI_rels_delay = (str[89] << 8) | str[90];		//S

		Cyclenpo.charge_OT_alrm = (((str[91] << 8) | str[92])*0.1)-50;		//C
		Cyclenpo.charge_OT_alrm_rels = (((str[93] << 8) | str[94])*0.1)-50;	//C
		Cyclenpo.charge_OT_alrm_delay = (str[95] << 8) | str[96];			//mS
		Cyclenpo.charge_OT_prtctn = (((str[97] << 8) | str[98])*0.1)-50;	//C
		Cyclenpo.charge_OT_prtctn_rels =(((str[99] << 8) | str[100])*0.1)-50;//C
		Cyclenpo.charge_OT_prtctn_delay = (str[101] << 8) | str[102];		//mS

		Cyclenpo.charge_UT_alrm = (((str[103] << 8) | str[104])*0.1)-50;	//C
		Cyclenpo.charge_UT_alrm_rels =(((str[105] << 8) | str[106])*0.1)-50;//C
		Cyclenpo.charge_UT_alrm_delay = (str[107] << 8) | str[108];			//mS
		Cyclenpo.charge_UT_prtctn = (((str[109] << 8) | str[110])*0.1)-50;	//C
		Cyclenpo.charge_UT_prtctn_rels =(((str[111] << 8) | str[112])*0.1)-50;//C
		Cyclenpo.charge_UT_prtctn_delay = (str[113] << 8) | str[114];		//mS

		Cyclenpo.discharge_OT_alrm = (((str[115] << 8) | str[116])*0.1)-50;	//C
		Cyclenpo.discharge_OT_alrm_rels = (((str[117] << 8) | str[118])*0.1)-50;//C
		Cyclenpo.discharge_OT_alrm_delay = (str[119] << 8) | str[120];		//mS
		Cyclenpo.discharge_OT_prtctn = (((str[121] << 8) | str[122])*0.1)-50;//C
		Cyclenpo.discharge_OT_prtctn_rels = (((str[123] << 8) | str[124])*0.1)-50;//C
		Cyclenpo.discharge_OT_prtctn_delay = (str[125] << 8) | str[126];	//mS

		Cyclenpo.discharge_UT_alrm = (((str[127] << 8) | str[128])*0.1)-50;	//C
		Cyclenpo.discharge_UT_alrm_rels = (((str[129] << 8) | str[130])*0.1)-50;//C
		Cyclenpo.discharge_UT_alrm_delay = (str[131] << 8) | str[132];		//mS
		Cyclenpo.discharge_UT_prtctn = (((str[133] << 8) | str[134])*0.1)-50;//C
		Cyclenpo.discharge_UT_prtctn_rels = (((str[135] << 8) | str[136])*0.1)-50;//C
		Cyclenpo.discharge_UT_prtctn_delay = (str[137] << 8) | str[138];	//mS

		Cyclenpo.MOS_HT_alrm = (((str[139] << 8) | str[140])*0.1)-50;		//C
		Cyclenpo.MOS_HT_alrm_rels = (((str[141] << 8) | str[142])*0.1)-50;	//C
		Cyclenpo.MOS_HT_alrm_delay = (str[143] << 8) | str[144];			//mS
		Cyclenpo.MOS_HT_prtctn = (((str[145] << 8) | str[146])*0.1)-50;		//C
		Cyclenpo.MOS_HT_prtctn_rels = (((str[147] << 8) | str[148])*0.1)-50;//C
		Cyclenpo.MOS_HT_prtctn_delay = (str[149] << 8) | str[150];			//mS

		Cyclenpo.Amb_HT_alrm = (((str[151] << 8) | str[152])*0.1)-50;		//C
		Cyclenpo.Amb_HT_alrm_rels = (((str[153] << 8) | str[154])*0.1)-50;	//C
		Cyclenpo.Amb_HT_alrm_delay = (str[155] << 8) | str[156];			//mS
		Cyclenpo.Amb_HT_prtctn = (((str[157] << 8) | str[158])*0.1)-50;		//C
		Cyclenpo.Amb_HT_prtctn_rels = (((str[159] << 8) | str[160])*0.1)-50;//C
		Cyclenpo.Amb_HT_prtctn_delay = (str[161] << 8) | str[162];			//mS

		Cyclenpo.Amb_LT_alrm = (((str[163] << 8) | str[164])*0.1)-50;		//C
		Cyclenpo.Amb_LT_alrm_rels = (((str[165] << 8) | str[166])*0.1)-50;	//C
		Cyclenpo.Amb_LT_alrm_delay = (str[167] << 8) | str[168];			//mS
		Cyclenpo.Amb_LT_prtctn = (((str[169] << 8) | str[170])*0.1)-50;		//C
		Cyclenpo.Amb_LT_prtctn_rels = (((str[171] << 8) | str[172])*0.1)-50;//C
		Cyclenpo.Amb_LT_prtctn_delay = (str[173] << 8) | str[174];			//mS

		Cyclenpo.OT_diffrnt_alrm = ((str[175] << 8) | str[176])*0.1;		//C
		Cyclenpo.OT_diffrnt_alrm_rels = ((str[177] << 8) | str[178])*0.1;	//C
		Cyclenpo.OT_diffrnt_alrm_delay = (str[179] << 8) | str[180];		//mS
		Cyclenpo.OT_diffrnt_prtctn = ((str[181] << 8) | str[182])*0.1;		//C
		Cyclenpo.OT_diffrnt_prtctn_rels = ((str[183] << 8) | str[184])*0.1;	//C
		Cyclenpo.OT_diffrnt_prtctn_delay = (str[185] << 8) | str[186];		//mS

		Cyclenpo.OV_diffrnt_alrm = (str[187] << 8) | str[188];				//mV
		Cyclenpo.OV_diffrnt_alrm_rels = (str[189] << 8) | str[190];			//mV
		Cyclenpo.OV_diffrnt_alrm_delay = (str[191] << 8) | str[192];		//mS
		Cyclenpo.OV_diffrnt_prtctn = (str[193] << 8) | str[194];			//mV
		Cyclenpo.OV_diffrnt_prtctn_rels = (str[195] << 8) | str[196];		//mV
		Cyclenpo.OV_diffrnt_prtctn_delay = (str[197] << 8) | str[198];		//mS

		Cyclenpo.SOC_too_low = (str[199] << 8) | str[200];					//%
		Cyclenpo.Low_SOC_alrm_rels = (str[201] << 8) | str[202];			//%
		Cyclenpo.Low_SOC_alrm_delay = (str[203] << 8) | str[204];			//mS
	}
			break;
			default:
				break;
			}
		}
}


int BP3_cyclenpobat_Send(void){
	//7E 32 35 30 32 34 36 34 32 45 30 30 32 30 32 46 44 32 45 0D
	static uint32_t supTick=0;
	static int state=0,state1=0;
	int status=0;
	switch(state)
	{
	case 0:
		BP3_cyclenpo_realtime_read();
		BP3_cyclenpo_batprtctn_read();
		state=1;
		break;
	case 1:
		if(HAL_GetTick()-supTick>5000)
	 {
			status=1;
			supTick=HAL_GetTick();
			if(RString[0]==0x11)
			{
				RString[0]=0x12;
				RString[6]=0xb7;
				RString[7]=0x97;
			}	else if(RString[0]==0x12)
			{
				RString[0]=0x11;
				RString[6]=0xb7;
				RString[7]=0xa4;
			}


				MAC_MailToMbs(RString,8);
			/*

			 else if(state1==1 && Process_complete==1){
				MAC_MailToMbs(PString,8);state1=0;state=2;Process_complete=0;}
				*/
	}
		break;

	case 2:
		status=1;
		if(send_ready==1){
		if(0==BP3_To_HMI_sendWriteReq())
		{
			status=0;
			supTick=HAL_GetTick();
			state=0;
			send_ready=0;
		}}
		break;
	}
	return status;
}

int BP3_To_HMI_sendWriteReq(void){

	/*
	static char mbStr[256];
	int status = 1;
	static int kindex = 7;
	static int state=0;
	int BP3ToHmimemoryMap[50]={0};
	switch(state)
	{
	case 0:
		BP3ToHmimemoryMap[0] =  (int) (Cyclenpo.Vtotal * 10);
		BP3ToHmimemoryMap[1] =  (int) (Cyclenpo.CurrentTotal * 10);
		BP3ToHmimemoryMap[2] =  (int) (Cyclenpo.SOC * 10);
		Cyclenpo.Remaining_Cap= (Cyclenpo.SOC*0.05);
		BP3ToHmimemoryMap[3] =  (int) (Cyclenpo.Remaining_Cap * 10);
		BP3ToHmimemoryMap[4] =  (int) (Cyclenpo.T_MOS * 10);
		BP3ToHmimemoryMap[5] =  (int) (Cyclenpo.T_amb * 10);
		BP3ToHmimemoryMap[6] =  (int) (Cyclenpo.Status * 10);
		BP3ToHmimemoryMap[7] =  (int) (Cyclenpo.Protection_code * 10);
		BP3ToHmimemoryMap[8] =  (int) (Cyclenpo.Warning_code * 10);
		BP3ToHmimemoryMap[9] =  (int) (Cyclenpo.T_max * 10);
		BP3ToHmimemoryMap[10] = (int) (Cyclenpo.T_min * 10);
		BP3ToHmimemoryMap[11] = (int) (Cyclenpo.T_cell[0] * 10);
		BP3ToHmimemoryMap[12] = (int) (Cyclenpo.T_cell[1] * 10);
		BP3ToHmimemoryMap[13] = (int) (Cyclenpo.T_cell[2] * 10);
		BP3ToHmimemoryMap[14] = (int) (Cyclenpo.T_cell[3] * 10);
		mbStr[0]= 0x02;
		mbStr[1] =  0x10;
		mbStr[2] = (BP3ToHmi_WRITE_START_ADDRESS & 0xff00) >> 8;
		mbStr[3] =  BP3ToHmi_WRITE_START_ADDRESS & 0x00ff;
		mbStr[4] = (BP3ToHmi_WRITE_NUMBER_BYTE & 0xff00) >> 8;
		mbStr[5] =  BP3ToHmi_WRITE_NUMBER_BYTE & 0x00ff;
		mbStr[6] = 2 * BP3ToHmi_WRITE_NUMBER_BYTE;
		kindex = 7;
		for (int index = 0; index < BP3ToHmi_WRITE_NUMBER_BYTE; index++) {
			mbStr[kindex++] = (((int) (BP3ToHmimemoryMap[index])) & 0xff00) >> 8;
			mbStr[kindex++] = ((int)  (BP3ToHmimemoryMap[index])) & 0x00ff;

		}
		uint16_t crc = calculateCRC(mbStr, kindex); //
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
	*/

}

