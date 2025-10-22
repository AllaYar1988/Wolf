
/*******************
 ***************************************** C SOURCE FILE ******************************************
 *******************/
/**
 *  @file       SMU_MNG_StateManager.c/.h
 *
 *  General Info
 *  ------------
 *  ____
 *  @par        File info
 *  @li @b      Version         : 1.0.0
 *  @li @b      Date            :
 *
 *  @par        Project info
 *  @li @b      Project         : SMU
 *  @li @b      Processor       : STM32f407
 *  @li @b      Tool  @b Chain  : CUBE IDE
 *  @li @b      Clock @b Freq   : 168 MHZ
 *
 *  @par        Description
 *              This file is generated in order to meet the requirements of ACU operating state module.
 *
 *  @copyright
 *
 **************************************************************************************************
 *  _______________
 *  Version History
 *  ---------------
 **************************************************************************************************
 *  ____
 *  @par        Rev 1.0.0
 *  @li @b      Date            : 5/14/2025
 *  @li @b      Author          : Allahyar Moazami
 *  @li @b      Approved @b by  :
 *  @li @b      Description
 *
 *              Revision Tag : Enter revision tag related to the current revision.
 *              Enter a paragraph that serves as a detail description.
 *
 **************************************************************************************************
 */

#include "SMU_MNG.h"
#include "adc.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "iwdg.h"
#include "rtc.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "Platform.h"
#include "SchCore.h"
#include "RTE_Task1kHz.h"
#include "WCET.h"
#include "ds1307_for_stm32_hal.h"
#include "cmd.h"
#include "sysvar.h"
#include "modbus.h"
#include "iflash.h"
#include "S1_INV.h"
#include "S2_CH.h"
#include "S5_VGBR.h"
#include "S3_HMI.h"
#include "dbg.h"
#include "mntdata.h"
#include "myrtc.h"
#include "crc.h"
#include "Ctrl.h"
#include "RTE_MNT_WEB.h"
#include "mac.h"
#include "smu.h"
#include "RTE_MB.h"
#include "BP_Mng.h"
#include "MEM.h"
#include "MBS.h"
#include "MBM.h"
#include "MdmSrv.h"
#include "MdmHw.h"
#include "inv_fault_recorder.h"




static StrECUStateMng_t StrECUStateMng = {ESM__INIT__STATE , ESM__INIT__STATE , MAX_NUM_OF_TRANSITION};

static void VoidInitStateMng(void);
static void VoidEcuMInit(void);
void VoidSwcAppInit(void) ;



/**
 ***************************************************************************************************
 *  \fn EnuECUState_t ESM_EnuECUStateManager(void)
 *
 *  \breif
 *	Checks the current ECU state and according to the current conditions decides about next state.
 *	This function shall be call every 250us
 *
 *	\return
 *	This function returns the current ACU operating state.
 *
 ***************************************************************************************************
 */
EnuECUState_t SMU_MNG_StateManager(void)
{
	/* Holding the Old ECU state for 250us */
	//StrECUStateMng.enuOldECUState = StrECUStateMng.enuNewECUState;

	switch (StrECUStateMng.enuNewECUState)
	{
	case ESM__INIT__STATE:
		VoidInitStateMng();
		break;

	case ESM__STARTUP__STATE:
		//VoidStartUpStateMng();
		break;

	case ESM__FULLY_OPERATIONAL__STATE:
		//VoidFullyOperationalStateMng();
		break;

	case ESM__WAKEUP__STATE:
		//VoidWakeupStateMng();
		break;

	case ESM__FAIL_SAFE__STATE:
		//VoidFailSafeStateMng();
		break;

	case ESM__EOL_UNLOCK__STATE:
		//VoidEOLUnlockStateMng();
		break;

	default:
		break;
	}

	/* PMode variable measurement management shall be done here */

	return StrECUStateMng.enuNewECUState;
}



/**
 ***************************************************************************************************
 *  \fn static void VoidInitStateMng(void)
 *
 *  \breif
 *	Checks the exclusion criteria from Init state.
 *	ECU can go to the below states from init state:
 *  - STARTUP
 *
 *  Cycle:
 *
 ***************************************************************************************************
 */
static void VoidInitStateMng(void)
{
	VoidEcuMInit();

	/* Analyzing the initialization errors and notify to DEM module about it */
	//EnuInitErrorHandling();

	/* Going to Start-up state is granted */
	//StrECUStateMng.enuNewECUState = ESM__STARTUP__STATE;
	//StrECUStateMng.enuECUStateTransition = INIT__TO__STARTUP;

	/* Start the scheduler with 500us tick */
	//SCP_VoidStartSch();

	/* Turn-Off AWL from auxiliary path,AWL auxilary is active low */
	//PORT_DIO_PUT_AWL_EN(PHYSICAL_LEVEL_HIGH);

	/* Enabling the cpu interrupt */
	// EI();
}


/**
 ***************************************************************************************************
 *  \fn static void VoidEcuMInit(void)
 *
 *  \breif
 *	 Is responsible for the initialization and de-initialization of all basic software modules
 *	 including OS and RTE.
 *
 ***************************************************************************************************
 */
static void VoidEcuMInit(void)
{
	/* Clear the first status of all errors in ordrt to update them during system initialization
	 phase */
	//UnionInitErr.int16uData = 0u;

	/*
	 **************************
	 * RL78 SELF-TEST PHASE2
	 **************************
	 */
	// if(ST_OK != STM_Int8uRL78SelfTestHandler())
	// {
	/* If there is a problem in RAM march test or another self-tests so ACU will be reset */
	//	  WDM_VoidBSWSysSrvWdgPerformReset	(WDM_ABNORMAL_RESET_RL78_CPU_TEST_FAILED);
	// }
	// else
	// {
	/*
	 ***********************
	 * MCAL INITIALIZATION
	 ***********************
	 */
	//VoidMcalInit();
	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	//MX_ADC1_Init();
	MX_I2C1_Init();
	MX_USART1_UART_Init();
	MX_USART3_UART_Init();
	MX_USART6_UART_Init();
	MX_SPI2_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM5_Init();
	MX_TIM4_Init();
	MX_RTC_Init();
	MX_IWDG_Init();
	MX_SDIO_SD_Init();
	MX_FATFS_Init();

	/*
	 *********************
	 * WCET
	 *********************
	 */
	WCET_InitDwt();
	/*
	 *********************
	 * NVM INITIALIZATION
	 *********************
	 */
	/* DEM and NVM first intialization */
	//DEM_VoidInit();
	//NVM_VoidInit();

	/*
	 * Read all of the system parameters from NVM
	 */
	//if (FAULT_IS_PRESENT == NVM_BoolReadParam())
	//{
	/* The relative error shall be stored */
	//		UnionInitErr.bitAccess.boolNvmInitErr = FAULT_IS_PRESENT;
	// }

	/*
	 *********************
	 * HAL INITIALIZATION
	 *********************
	 */
	//VoidHalInit();

	/*
	 ***********************************
	 * SYSTEM MANAGEMENT INITIALIZATION
	 ***********************************
	 */
	//VoidSystemMangtInit();

	/*
	 ***********************
	 * SW-Cs INITIALIZATION
	 ***********************
	 */
	//VoidSwcAppInit();
	VoidSwcAppInit();
	//}
}

/**
 ***************************************************************************************************
 *  \fn void VoidSwcAppInit(void)
 *
 *  \breif
 *	 Is responsible for the initialization and de-initialization of all basic software modules
 *	 including OS and RTE.
 *
 ***************************************************************************************************
 */
void VoidSwcAppInit(void) {
	// Buffer to read the string back
	char read_string[20] = { 0 };
	mcuRstMsg();
	memInit();
	// Read the string from flash memory
	Flash_Read_String(FLASH_USER_START_ADDR, read_string, sizeof(read_string));
	if (strstr(read_string, "UA")) {
		strcpy(_serialN, read_string);
	}

	SCC_Int8uAddTask( RTE_Task1KHz,0,1);
	SMU_Slaves_Database_Init();
	HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_1);
	HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_OC_Start(&htim5, TIM_CHANNEL_1);

	//HAL_UART_Receive_DMA(&huart1, (uint8_t*) _data1, 1); //GSM
	HAL_Delay(100);

	// Initialize RTC
	DS1307_Init(&hi2c1);
	HAL_Delay(2000);



	_rtcFunctionRead(0);


	registerCommand("dbg", debugCommand,debugHelp);
	registerCommand("version", returnVersion,returnVersionHelp);
	registerCommand("clk", clkCommand,clkCommandHelp);
	registerCommand("serial", serialCommand,serialCommandHelp);
	registerCommand("help", helpCommand,helpHelp);
	registerCommand("mcu", mcuControl,mcuControlHelp);
	registerCommand("chcom", BP_MngCommunication,BP_MngCommunicationHelp);
	registerCommand("REF", refSetCommand,refSetCommandHelp);
	registerCommand("sim", simCommand,simCommandHelp);
	registerCommand("AT", atDirectCommand,atDirectCommandHelp);
	registerCommand("ievent", inv_fault_recorder_cmd,inv_fault_recorder_cmd_help);

	//registerCommand("WCET", wcetCommand,wcetCommandHelp);

	MODEM_POWER(ON)
}


