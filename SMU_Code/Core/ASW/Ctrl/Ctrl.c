/*******************
 ***************************************** C SOURCE FILE ******************************************
 *******************/
/**
 *  @file       CTRL.C
 *
 *  General Info
 *  ------------
 *  ____
 *  @par        File info
 *  @li @b      Version         : 1.0.4
 *  @li @b      Date            : 2024-05-14
 *
 *  @par        Project info
 *  @li @b      Project         : xxxxx
 *  @li @b      Processor       : STM32f4xx
 *  @li @b      Tool  @b Chain  :
 *  @li @b      Clock @b Freq   : 168 MHz
 *
 *  @par        This file create for Control Algorithms for SMU
 *
 *  @copyright  (C)
 *
 **************************************************************************************************
 *  _______________
 *  Version History
 *  ---------------
 **************************************************************************************************
 *
 *              This veraion has been tested.
 *  ____
 *  @par        Rev 1.0.0
 *  @li @b      Date            : 2025-05-04
 *  @li @b      Author          : A.Moazami
 *  @li @b      Approved @b by  :
 *  @li @b      Description
 *
 *              Initial Version
 *
 **************************************************************************************************
 */

#include "Ctrl.h"
#include "mntdata.h"
#include "modbus.h"
#include "S2_CH.h"
#include "S1_INV.h"
#include "smu.h"
#include "BP1_Batt.h"

/*!
 **************************************************************************************************
 *
 *  @fn         Global Private Var Define
 *
 **************************************************************************************************
 */
int CHcommand = 1;
int INVcommand = 1;
int MODESEL = 0, GENOFF = 0, DEADBAT = 0;
int USSR1 = 1, URELAY1 = 1;
int USSR2 = 0, URELAY2 = 0, URELAY3 = 0, URELAY4 = 0;
int TurnONDelay = 0;

int LEDr = 0, LEDb = 0, LEDg = 0;
LED_Color_t LED_COLOR = LED_SOLID_OFF;
u16 U16BattState=0;

/*!
 **************************************************************************************************
 *
 *  @fn         Private Function Declaration
 *
 **************************************************************************************************
 */
static void Ctrl_SetOutputs(void) ;
static void Ctrl_OutputRoutine(void) ;
static void Ctrl_CheckConditions(void);
static void Set_LED_Color(LED_Color_t color) ;

/*!
 **************************************************************************************************
 *
 *  @fn         Ctrl_Handler (void)
 *
 *  @par        Contains all Control Algorithms.
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
void Ctrl_Handler(void)
{
	static int cnt=0;
	static int cnt1s=0;
_memoryMap[241]=U16BattState;
_memoryMap[242] = URELAY1;
_memoryMap[243] = URELAY2;
_memoryMap[244] = URELAY3;
_memoryMap[245] = URELAY4;
_memoryMap[246] = USSR1;
_memoryMap[247] = USSR2;
_memoryMap[248] = USSR1;
_memoryMap[249] = USSR2;
u32 u32Tick=HAL_GetTick();
static u32 stcU32Tick=0;
if((u32Tick-stcU32Tick)>100)
{
	Ctrl_SetOutputs();
	Ctrl_CheckConditions();
	stcU32Tick=u32Tick;

	cnt=0;
}
if(cnt1s++>100){
	Set_LED_Color(LED_COLOR);

	cnt1s=0;
}
}
/*!
 **************************************************************************************************
 *
 *  @fn         static void Ctrl_SetOutputs (void)
 *
 *  @par        Sets Output
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
static void Ctrl_SetOutputs(void) {
	//Grid SSR
	if (USSR1 == 1) {
		HAL_GPIO_WritePin(uDo1_GPIO_Port, uDo1_Pin, OptoOn);
		HAL_GPIO_WritePin(uRelayGen_GPIO_Port, uRelayGen_Pin, RelayOn);
	} else {
		HAL_GPIO_WritePin(uDo1_GPIO_Port, uDo1_Pin, OptoOff);
		HAL_GPIO_WritePin(uRelayGen_GPIO_Port, uRelayGen_Pin, RelayOff);
	}

	/*	if (URELAY2 == 1)
		HAL_GPIO_WritePin(uRelay3_GPIO_Port, uRelay3_Pin, RelayOn);
	else
		HAL_GPIO_WritePin(uRelay3_GPIO_Port, uRelay3_Pin, RelayOff);*/

	/*
	 if(_memoryMap[25]==1){HAL_GPIO_WritePin(uDo2_GPIO_Port,uDo2_Pin,GPIO_PIN_SET);
	 HAL_GPIO_WritePin(uRelay4_GPIO_Port,uRelay4_Pin,RelayOn);
	 }
	 else
	 {HAL_GPIO_WritePin(uDo2_GPIO_Port,uDo2_Pin,GPIO_PIN_RESET);
	 HAL_GPIO_WritePin(uRelay4_GPIO_Port,uRelay4_Pin,RelayOff);
	 }
	 */
	//Gen start Relay
	if (URELAY1 == 1)
		HAL_GPIO_WritePin(uRelay1_GPIO_Port, uRelay1_Pin, RelayOff);
	else
		HAL_GPIO_WritePin(uRelay1_GPIO_Port, uRelay1_Pin, RelayOn);
	/*
	if (URELAY3 == 1)
		HAL_GPIO_WritePin(uRelay2_GPIO_Port, uRelay2_Pin, RelayOn);
	else
		HAL_GPIO_WritePin(uRelay2_GPIO_Port, uRelay2_Pin, RelayOff);
	 */
	//	HAL_GPIO_WritePin(uRelayLoad_GPIO_Port, uRelayLoad_Pin, RelayOn);
	//Grid relay logic is reverse of load and gen relay. To turn off: uRelayGridP_Pin =1 and uRelayGridN_Pin = 0
	HAL_GPIO_WritePin(uRelayGridP_GPIO_Port, uRelayGridP_Pin, 1);
	HAL_GPIO_WritePin(uRelayGridN_GPIO_Port, uRelayGridN_Pin, 0);
	//	HAL_GPIO_WritePin(uRelayGen_GPIO_Port, uRelayGen_Pin, RelayOn);


}


/*!
 **************************************************************************************************
 *
 *  @fn         static void Ctrl_OutputRoutine (void)
 *
 *  @par        Controls the Output Relay and SSR Based On State Machine
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
static void Ctrl_OutputRoutine(void) {
	// Main logic routine
	Database_Type* mDb = getMntDatabase();

	if ((DEADBAT == 0) && !((int) mDb[1].mData[S1_MNT_InvState].value == 3) && !((int) mDb[2].mData[S2_MNT_State].value == 3)) {
		if ((MODESEL == 0)||(MODESEL == 2)) {
			if (GENOFF == 0) {
				if (INVcommand == 1) {
					if (CHcommand == 1) {
						USSR1 = 1;
						URELAY1 = 1;
					} else { // CHcommand == 0
						USSR1 = 0;
						URELAY1 = 0;
					}
				} else if (INVcommand == 0) {
					if (CHcommand == 1) {
						USSR1 = 0;
						URELAY1 = 1;
					} else { // CHcommand == 0
						USSR1 = 0;
						URELAY1 = 0;
					}
				}
			} else { // GENOFF != 0
				USSR1 = 0;
				URELAY1 = 0;
			}
		} else { // Backup mode when MODESEL != 0
			if (GENOFF == 0) {
				if (INVcommand == 1) {
					USSR1 = 1;
				} else { // INVcommand == 0
					USSR1 = 0;
				}
				if (CHcommand == 1) {
					URELAY1 = 1;
				} else { // CHcommand == 0
					URELAY1 = 0;
				}
			} else { // GENOFF != 0
				USSR1 = 0;
				URELAY1 = 0;
			}
		}
	} else {
		USSR1 = 1;
		URELAY1 = 1;
	}
}

/*!
 **************************************************************************************************
 *
 *  @fn         static void Ctrl_CheckConditions (void)
 *
 *  @par        Checks Working Condition
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
static void Ctrl_CheckConditions(void)
{  Database_Type* mDb = getMntDatabase();

static int countssroff = 0, countssron = 0;
static int countgenoff = 0, countgenon = 0;
if ((SSR_ON & 0x01) == 1) {
	if (countssron++ > 10) {
		INVcommand = SSR_ON & 0x01;
		countssron = 11;
		countssroff = 0;
	}
} else {
	if (countssroff++ > 20) {
	INVcommand = SSR_ON & 0x01;
	countssroff = 21;
	countssron = 0;
	}
}
if ((GEN_ON & 0x01) == 1) {
	if (countgenon++ > 10) {
		CHcommand = GEN_ON & 0x01;
		countgenon = 11;
		countgenoff = 0;
	}
} else {
	if (countgenoff++ > 20) {
		CHcommand = GEN_ON & 0x01;
		countgenoff = 21;
		countgenon = 0;
	}
}

_memoryMap[232] = INVcommand;
_memoryMap[233] = CHcommand;

//Set outputs
Ctrl_OutputRoutine();
MODESEL = _memoryMap[1];
GENOFF = _memoryMap[37];
}

/*!
 **************************************************************************************************
 *
 *  @fn         static void Ctrl_GeneralControl (void)
 *
 *  @par        Performs some general control functions based on Temp and Vbat
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
void Ctrl_GeneralControl(void)
{
	Database_Type* mDb = getMntDatabase();

	MntDataType* mData = mDb[1].mData;
	float f32BattPower=S2_CH_GetBattPower();
	float f32TempRef = (float) _memoryMap[30] / 10;
	u16 u16DeadBattState=(u16)mDb[1].mData[S1_MNT_InvState].value;
	RefDataType *rData=getRefData();
	if(rData[INV_LOAD_RLY_INDEX].value == 2)
	{
		HAL_GPIO_WritePin(GPIOD, uRelayLoad_Pin, GPIO_PIN_SET);

	}
	else
	{
		HAL_GPIO_WritePin(GPIOD, uRelayLoad_Pin, GPIO_PIN_RESET);
	}



	mData = mDb[0].mData;
	mData[SMU_MNT_TEMPE].value=getTemp();
	_memoryMap[236] = (int) (getTemp() * 10);
	BatteryPack1.Vblock = (float) _memoryMap[35] / 100;
	BatteryPack1.Tblock = _memoryMap[36];

	readTempSensor(_memoryMap[32]);

	HAL_Delay(1000);

	if (TurnONDelay++ > 20) {
		if (_tempFinal < f32TempRef) {
			HAL_GPIO_WritePin(uRelay5_GPIO_Port, uRelay5_Pin,
					RelayOn);
			_memoryMap[237] = 1;
		}
		if (_tempFinal > f32TempRef + _memoryMap[31]) {
			HAL_GPIO_WritePin(uRelay5_GPIO_Port, uRelay5_Pin,
					RelayOff);
			_memoryMap[237] = 0;
		}
		/*	if ((_memoryMap_bchF[1] > 30.0)) {
					if (BatteryPack1.Temp1 < BatteryPack1.Tref) {
						HAL_GPIO_WritePin(uRelay5_GPIO_Port, uRelay5_Pin,
								RelayOn);
						_memoryMap[237] = 1;
					}
					if (BatteryPack1.Temp1 > BatteryPack1.Tref + _memoryMap[34]) {
						HAL_GPIO_WritePin(uRelay5_GPIO_Port, uRelay5_Pin,
								RelayOff);
						_memoryMap[237] = 0;
					}
				}
		 */

		/*if (_memoryMap_bchF[1] < BatteryPack1.Vblock + 1) {
					if ((_tcount < ((60 * BatteryPack1.Tblock) + 10))
							&& (SSR_ON & 0x01) == 0) {URELAY3 = 1;
					} else {
						URELAY3 = 0;
					}

				} else
					_tcount = 0;*/

		TurnONDelay = 21;
	}

	//System status update - = discharge


	if ((f32BattPower > BATT_POWER_LOW_HYST) && (f32BattPower < BATT_POWER_HIGH_HYST)) {
		U16BattState = BATT_STATE_IDLE;
		LED_COLOR = LED_SOLID_GREEN;
	} //Idle
	else if (f32BattPower > BATT_POWER_HIGH_HYST) {
		U16BattState = BATT_STATE_CHARGING;
		LED_COLOR = LED_BLINK_GREEN;
	} //charge
	else if (f32BattPower < BATT_POWER_LOW_HYST) {
		U16BattState = BATT_STATE_DISCHARGING;
		LED_COLOR = LED_SOLID_GREEN;
	} //Discharge
	if (u16DeadBattState  == BATTERY_DEAD) {
		U16BattState = BATT_STATE_DEAD_RCOVERY;
		LED_COLOR = LED_BLINK_RED;
	} //Dead batt recovery
	// _memoryMap[241] = 5; // Fault

	//HAL_GPIO_WritePin(Slave_Select_GPIO_Port,Slave_Select_Pin,0);

	// if(_memoryMap[28]==1)HAL_GPIO_WritePin(uRelay3_GPIO_Port,uRelay3_Pin,1);
	// else HAL_GPIO_WritePin(uRelay3_GPIO_Port,uRelay3_Pin,0);

	//  if(_memoryMap[29]==1)HAL_GPIO_WritePin(uRelay4_GPIO_Port,uRelay4_Pin,1);
	// else HAL_GPIO_WritePin(uRelay4_GPIO_Port,uRelay4_Pin,0);

}
/*!
 **************************************************************************************************
 *
 *  @fn         void Set_LED_Color(LED_Color_t color)
 *
 *  @par
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
static void Set_LED_Color(LED_Color_t color)
{
	static bool LEDr = LED_OFF;
	static bool LEDg = LED_OFF;
	static bool LEDb = LED_OFF;

	switch (color) {
	case LED_SOLID_OFF:
		LEDr = LED_OFF;
		LEDg = LED_OFF;
		LEDb = LED_OFF;
		break;
	case LED_SOLID_RED:
		LEDr = LED_ON;
		LEDg = LED_OFF;
		LEDb = LED_OFF;
		break;
	case LED_SOLID_GREEN:
		LEDr = LED_OFF;
		LEDg = LED_ON;
		LEDb = LED_OFF;
		break;
	case LED_SOLID_BLUE:
		LEDr = LED_OFF;
		LEDg = LED_OFF;
		LEDb = LED_ON;
		break;
	case LED_BLINK_RED:
		LEDr = 1-LEDr;
		LEDg = LED_OFF;
		LEDb = LED_OFF;
		break;
	case LED_BLINK_GREEN:
		LEDr = LED_OFF;
		LEDg = 1-LEDg;
		LEDb = LED_OFF;
		break;
	case LED_BLINK_BLUE:
		LEDr = LED_OFF;
		LEDg = LED_OFF;
		LEDb = 1-LEDb;
		break;
	default:
		// Handle undefined color
		LEDr = LED_OFF;
		LEDg = LED_OFF;
		LEDb = LED_OFF;
		break;
	}

	// Write to the GPIOs
	HAL_GPIO_WritePin(uRGB_R_GPIO_Port, uRGB_R_Pin, LEDr);
	HAL_GPIO_WritePin(uRGB_G_GPIO_Port, uRGB_G_Pin, LEDg);
	HAL_GPIO_WritePin(uRGB_B_GPIO_Port, uRGB_B_Pin, LEDb);
}


