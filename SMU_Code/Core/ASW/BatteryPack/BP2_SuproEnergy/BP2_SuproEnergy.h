/*
 * BP2_SuproEnergy.h
 *
 *  Created on: Feb 11, 2025
 *      Author: 98912
 */

#ifndef ASW_BATTERYPACK_BP2_SUPROENERGY_BP2_SUPROENERGY_H_
#define ASW_BATTERYPACK_BP2_SUPROENERGY_BP2_SUPROENERGY_H_
#define BP2_SUPRO_ADDRESSS 0x01
#define BP2ToHmi_WRITE_START_ADDRESS 800
#define BP2ToHmi_WRITE_NUMBER_BYTE 20
#include "Platform.h"

typedef struct Supro_HandleTypeDef {
    int Slave_add;
    float Vtotal; //Total voltage
    float V_busbar; //Busbar return voltage
    float CurrentTotal; //Total current
    int SOC;
    float Remaining_Cap; //Remaining capacity
    float Full_cap; //Full charge capacity
    float Nom_cap; //Nominal capacity
    float T_MOS; //MOStemperature
    float T_amb; //Ambient temperature
    char Status; //Charge and discharge status
    int SOH;
    long int Protection_code; //Protection Information
    long int Warning_code; //warning Information
    int MOS_state;
    int Signal_status;
    int Cycle_times;
    int Max_mono; //Highest monomer number
    float Max_V_cell; //Maximum single cell voltage
    int Min_mono; //Minimum monomer number
    float Min_V_cell; //Minimum single cell voltage
    float V_ave; //Average voltage
    int T_max_SN; //Maximum temperature serial number
    float T_max; //Maximum temperature value
    int T_min_SN; //Minimum temperature serial number
    float T_min; //Minimum temperature value
    float T_ave; //Average Temperature
    float V_max_charge; //Maximum allowable charging voltage
    float I_max_charge; //Maximum allowable charging current
    float V_max_discharge; //Maximum allowable discharge voltage
    float I_max_discharge; //Maximum allowable discharge current
    int Num_Mono_N; //Number of monomers N
    float V_cell[30]; //Battery cell voltage 01 ~ N (Not sure about the max number of cells)
    int Num_Temp; //Temperature number M
    float T_cell[20]; //Battery temperature 01 ~ M
}Supro_HandleTypeDef;

extern int16_t _memoryMap[500];
extern Supro_HandleTypeDef SuproEnergy1;

int BP2_SuperoEnergySend(void);
void BP2_SuproEnergyResProcess(char *str,int Len);
#endif /* ASW_BATTERYPACK_BP2_SUPROENERGY_BP2_SUPROENERGY_H_ */
