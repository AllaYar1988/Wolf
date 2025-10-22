/*
 * Cyclenpo_bat.h
 *
 *  Created on: Feb 11, 2025
 *      Author: 98912
 */




#ifndef ASW_BATTERYPACK_BP3_CYCLENPO_BP3_CYCLENPO_H_
#define ASW_BATTERYPACK_BP3_CYCLENPO_BP3_CYCLENPO_H_

#define BP3_CYCLENPO_ADDRESSS 0x11
#define BP3_REALTIME_ADDRESSS 0x04
#define BP3_PROTECTION_ADDRESSS 0x03

#define BP3ToHmi_WRITE_START_ADDRESS 800
#define BP3ToHmi_WRITE_NUMBER_BYTE 20


typedef struct Cyclenpo_HandleTypeDef {
	int Slave_add;
	/*///////////// battery realtime parameters///////////////////*/
	float Vtotal; 					//Battery voltage(Battery voltage
	float CurrentTotal; 			//Total current
	int SOC; 						//SOC(SOC
	int SOH;						//SOHSOH
	int Max_mono; 					//Highest monomer number(Maximum cell No.
	float Max_V_cell; 				//Maximum single cell voltage(Maximum cell voltage
	int Min_mono; 					//Minimum monomer number(Minimum cell No.
	float Min_V_cell; 				//Minimum single cell voltageMinimum cell voltage
	int T_max_SN; 					//Maximum temperature serial number(Maximum temperature No.
	float T_max; 					//Maximum temperature value (Maximum temperature
	int T_min_SN; 					//Minimum temperature serial number (Minimum temperature No.
	float T_min; 					//Minimum temperature value (Minimum temperature
	float T_amb; 					//Ambient temperature (Ambient temperature
	float T_MOS; 					//MOStemperature (MOS temperature
	char Status; 					//Charge and discharge status (State of system
	float Full_cap; 				//Full charge capacity(Full capacity
	float Remaining_Cap; 			//Remaining capacity
	float Surplus_cap; 				//Surplus capacity
	long int Protection_code; 		//Protection Information (Protection of information
	long int Warning_code; 			//warning Information (Alarm of information
	char SN_Code[20];				//serial number
	float V_max_charge;				//Maximum allowable charging voltage//Charge voltage limit
	float I_max_charge; 			//Maximum allowable charging current //Charge current limit
	float V_max_discharge; 			//Maximum allowable discharge voltage //Discharge voltage limit
	float I_max_discharge; 			//Maximum allowable discharge current //Discharge current limit
	int fill_time;					//Estimated fill time
	int time_of_release;				//Estimated time of release
	int parallel_num;				//slave 0-16, 0 offline , 1 : on line.Parallel number
	int On_line_parallel;    		//on line parallel
	int Cycle_times;				//Cycle
	char Running_status;				//sleep, self check,running etc
	int Num_Mono_N; 				//Number of monomers N (Number of cells
	float V_cell[32]; 				//Battery cell voltage 01 ~ N (Not sure about the max number of cells)(Cell Voltage
	int Num_Temp; 					//Temperature number M (Number of temperature
	float T_cell[20]; 				//Battery temperature 01 ~ Mtemperature Value

	/*////////////////Battery Protection Parameter//////////////////*/

	float OV_cell_alrm;				//Cell over voltage alarm value
	float OV_cell_alrm_rels;		//Cell over voltage alarm release value
	int OV_cell_alrm_delay;			//Cell over voltage alarm delay
	float OV_cell_prtctn;			//Cell over voltage protection value
	float OV_cell_prtctn_rels;		//Cell over voltage protect release value
	int OV_cell_prtctn_delay;		//Cell over voltage protect delay

	float UV_cell_alrm;				////Cell under voltage alarm
	float UV_cell_alrm_rels;		//Cell under voltage alarm release value
	int UV_cell_alrm_delay;			//Cell under voltage alarm delay
	float UV_cell_prtctn;			//Cell under voltage protect value
	float UV_cell_prtctn_rels;		//Cell under voltage protect release value
	int UV_cell_prtctn_delay;		//Cell under voltage protect delay

	float OV_pack_alrm;				//Battery over voltage alarm value
	float OV_pack_alrm_rels;		//Battery over voltage alarm release value
	int OV_pack_alrm_delay;			//Battery over voltage alarm delay
	float OV_pack_prtctn;			//Battery over voltage protect value
	float OV_pack_prtctn_rels;		//Battery over voltage protect release value
	int OV_pack_prtctn_delay;		//Battery over voltage protect delay

	float UV_pack_alrm;				//Battery under voltage alarm value
	float UV_pack_alrm_rels;		//Battery under voltage alarm release value
	int UV_pack_alrm_delay;			//Battery under voltage alarm delay
	float UV_pack_prtctn;			//Battery under voltage protect value
	float UV_pack_prtctn_rels;		//Battery under voltage protect release value
	int UV_pack_prtctn_delay;		//Battery under voltage protect delay

	float charge_OI_alrm;			//Charge over current alarm value
	float charge_OI_alrm_rels;		//Charge over current alarm release value
	int charge_OI_alrm_delay;		//Charge over current alarm delay
	float charge_OI_prtctn;			//Charge over current protect value
	int charge_OI_prtctn_delay;		//Charge over current protect release value
	int charge_OI_rels_delay;		//Charge over current protect delay
	int charge_OI_lock_times;		//Charge over current lock times

	float charge2_OI_prtctn;		//Charge 2 over current protect value
	int charge2_OI_prtctn_delay;	//Charge 2 over current protect delay
	int charge2_OI_rels_delay;		//Charge 2 over current release delay

	float discharge_OI_alrm;		//Discharge over current alarm value
	float discharge_OI_alrm_rels;	//Discharge over current alarm release value
	int discharge_OI_alrm_delay;	//Discharge over current alarm delay
	float discharge_OI_prtctn;		//Discharge over current protect value
	int discharge_OI_prtctn_delay;	//Discharge over current protect delay
	int discharge_OI_rels_delay;	//Discharge over current release delay
	int discharge_OI_lock_times;	//Discharge over current lock times

	float discharge2_OI_prtctn;		//Discharge 2 over current protect value
	int discharge2_OI_prtctn_delay;	//Discharge 2 over current protect delay
	int discharge2_OI_rels_delay;	//Discharge 2 over current release delay

	float charge_OT_alrm;			//Charge over temperature alarm value
	float charge_OT_alrm_rels;		//Charge over temperature alarm release value
	int charge_OT_alrm_delay;		//Charge over temperature alarm delay
	float charge_OT_prtctn;			//Charge over temperature protect
	float charge_OT_prtctn_rels;	//Charge over temperature protection release
	int charge_OT_prtctn_delay;		//Charge over temperature protect delay

	float charge_UT_alrm;			//Charge under temperature alarm value
	float charge_UT_alrm_rels;		//Charge under temperature alarm release value
	int charge_UT_alrm_delay;		//Charge under temperature alarm delay
	float charge_UT_prtctn;			//Charge under temperature protect
	float charge_UT_prtctn_rels;		//Charge under temperature protect release
	int charge_UT_prtctn_delay;		//Charge under temperature protect delay

	float discharge_OT_alrm;		//Discharge over temperature alarm value
	float discharge_OT_alrm_rels;	//Discharge over temperature alarm release value
	int discharge_OT_alrm_delay;	//Discharge over temperature alarm delay
	float discharge_OT_prtctn;			//Discharge over temperature protect delay
	float discharge_OT_prtctn_rels;	//Discharge over temperature protect release
	int discharge_OT_prtctn_delay;	//Discharge over temperature protect delay

	float discharge_UT_alrm;		//Discharge under temperature alarm value
	float discharge_UT_alrm_rels;	//Discharge under temperature alarm release value
	int discharge_UT_alrm_delay;	//Discharge under temperature alarm delay
	float discharge_UT_prtctn;		//Discharge under temperature protect
	float discharge_UT_prtctn_rels;	//Discharge under temperature protect release
	int discharge_UT_prtctn_delay;	//Discharge under temperature protect delay

	float MOS_HT_alrm;				//MOS high temperature alarm value
	float MOS_HT_alrm_rels;			//MOS high temperature alarm release value
	int MOS_HT_alrm_delay;			//MOS high temperature alarm delay
	float MOS_HT_prtctn;			//MOS high temperature protect
	float MOS_HT_prtctn_rels;		//MOS high temperature protect release
	int MOS_HT_prtctn_delay;		//MOS high temperature protect delay

	float Amb_HT_alrm;				//Ambient high temperature alarm value
	float Amb_HT_alrm_rels;			//Ambient high temperature alarm release value
	int Amb_HT_alrm_delay;			//Ambient high temperature alarm delay
	float Amb_HT_prtctn;			//Ambient high temperature protect
	float Amb_HT_prtctn_rels;		//Ambient high temperature protect release
	int Amb_HT_prtctn_delay;		//Ambient high temperature protect delay

	float Amb_LT_alrm;				//Ambient low temperature alarm value
	float Amb_LT_alrm_rels;			//Ambient low temperature alarm release value
	int Amb_LT_alrm_delay;			//Ambient low temperature alarm delay
	float Amb_LT_prtctn;			//Ambient low temperature protect
	float Amb_LT_prtctn_rels;		//Ambient low temperature protect release
	int Amb_LT_prtctn_delay;		//Ambient low temperature protect delay

	float OT_diffrnt_alrm;			//Over temperature different alarm value
	float OT_diffrnt_alrm_rels;		//Over temperature different alarm release value
	int OT_diffrnt_alrm_delay;		//Over temperature different alarm delay
	float OT_diffrnt_prtctn;		//Over temperature different protect
	float OT_diffrnt_prtctn_rels;	//Over temperature different protect release
	int OT_diffrnt_prtctn_delay;	//Over temperature different protect delay

	float OV_diffrnt_alrm;			//Over voltage different alarm value
	float OV_diffrnt_alrm_rels;		//Over voltage different alarm release value
	int OV_diffrnt_alrm_delay;		//Over voltage different alarm delay
	float OV_diffrnt_prtctn;		//Over voltage different protect
	float OV_diffrnt_prtctn_rels;	//Over voltage different protect release
	int OV_diffrnt_prtctn_delay;	//Over voltage different protect delay

	int SOC_too_low;				//The SOC is too low
	int Low_SOC_alrm_rels;			//Low SOC alarm release value
	int Low_SOC_alrm_delay;			//Low SOC alarm delay


}Cyclenpo_HandleTypeDef;

extern Cyclenpo_HandleTypeDef Cyclenpo;
extern int Process_complete;
extern int send_ready;
int BP3_cyclenpobat_Send(void);
void Cyclenpo_realtime_Process(char *str,int Len);
void Cyclenpo_batprtctn_Process(char *str,int Len);
#endif /* ASW_BATTERYPACK_BP2_SUPROENERGY_BP2_SUPROENERGY_H_ */
