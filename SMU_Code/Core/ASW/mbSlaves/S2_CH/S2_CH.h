                                    /*******************
 ***************************************** C SOURCE FILE ******************************************
                                        *******************/
/**
 *  @file       S2_CH.C.H
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
 *  @li @b      Date            : 2025-05-30
 *  @li @b      Author          : A.Moazami
 *  @li @b      Approved @b by  :
 *  @li @b      Description
 *
 *              Initial Version
 *
 **************************************************************************************************
 */
#ifndef ASW_S2_CH_S2_CH_H_
#define ASW_S2_CH_S2_CH_H_
#include "modbus.h"
#include "Platform.h"

#define S2_CH_ID 0x21
#define S2_READ_START_ADDRESS  0
#define S2_READ_NUMBER_BYTE   30
#define S2_WRITE_START_ADDRESS  100
#define S2_WRITE_NUMBER_BYTE   20

/*State*/
#define READ_TIMEOUT  100
#define WRITE_TIMEOUT 101
/*Value*/
#define S2_WRITE_TIMEOUT_VALUE 200
#define S2_READ_TIMEOUT_VALUE  200

#define S2_WRITE_BUFFER_SIZE 256


typedef enum{
	S2_MNT_VDC_CH=0,   /*0*/
	S2_MNT_VBat_CH,    /*1*/
	S2_MNT_IBat_CH,    /*2*/
	S2_MNT_IBRI1C,     /*3*/
	S2_MNT_IBRI2C,     /*4*/
	S2_MNT_POWER1,     /*5*/
	S2_MNT_GenS,       /*6*/
	S2_MNT_State,      /*7*/
	S2_MNT_FCC,       /*8*/
	S2_MNT_REFID,
        S2_MNT_DATA_Size
}S2_MntEnumType;

typedef struct{
uint32_t readTimeout;
uint32_t writeTimeout;
}S2_CH;


extern int16_t _memoryMap[500];

/*!
 **************************************************************************************************
 *
 *  @fn        void S2_CH_registerToMbNode(void)
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
void S2_CH_registerToMbNode(void);
/*!
 **************************************************************************************************
 *
 *  @fn         void S2_CH_Database_Init()
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
void S2_CH_Database_Init();
/*!
 **************************************************************************************************
 *
 *  @fn         float S2_CH_GetBattVoltage(void)
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
float S2_CH_GetBattPower(void);
/*!
 **************************************************************************************************
 *
 *  @fn         int  S2_CH_MbMng(void)
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
int S2_CH_MbMng(void);

/*!
 **************************************************************************************************
 *
 *  @fn          void S2_CH_setUpdateStatus(u16 u16Vlaue)
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
void S2_CH_setUpdateStatus(u16 u16Vlaue);
/*!
 **************************************************************************************************
 *
 *  @fn          u16 S2_CH_getUpdateStatus(void)
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
u16 S2_CH_getUpdateStatus(void);


#endif /* ASW_S2_CH_S2_CH_H_ */
