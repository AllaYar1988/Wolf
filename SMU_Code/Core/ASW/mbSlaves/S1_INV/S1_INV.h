                                    /*******************
 ***************************************** C SOURCE FILE ******************************************
                                        *******************/
/**
 *  @file       S1_INV.C.H
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

#ifndef ASW_S1_INV_S1_INV_H_
#define ASW_S1_INV_S1_INV_H_

#include "Platform.h"
#include "modbus.h"

#define S1_INV_ID 0x11
#define S1_READ_START_ADDRESS  20100
#define S1_READ_FR_START_ADDRESS  20200

#define MODEL_CSTM_1_ID 20100
#define MODEL_CSTM_2_ID 20200

#define S1_READ_NUMBER_BYTE   40
#define S1_WRITE_START_ADDRESS  20100
#define S1_WRITE_NUMBER_BYTE   40
/*State*/
#define READ_TIMEOUT  100
#define WRITE_TIMEOUT 101
/*Value*/
#define S1_WRITE_TIMEOUT_VALUE 200
#define S1_READ_TIMEOUT_VALUE  200

#define S1_WRITE_BUFFER_SIZE 256


#define BATTERY_DEAD_PREF -0.5f
#define SCALE_FACTOR_10   0.1f

typedef enum{
	S1_MNT_VDC=0,/*1*/
	S1_MNT_Iinv1,/*2*/
	S1_MNT_Iinv2,/*3*/
	S1_MNT_VO1,/*4*/
	S1_MNT_VO2,/*5*/
	S1_MNT_VG1,/*6*/
	S1_MNT_VG2,/*7*/
	S1_MNT_FRQI,/*8*/
	S1_MNT_SSRS,/*9*/
	S1_MNT_FCODE,/*10*/
	S1_MNT_FTRIG,/*11*/
    S1_MNT_Tempinv, /*12*/
    S1_MNT_InvState,  /*13*/
	S1_MNT_REFID,
    S1_MNT_DATA_Size
}S1_MntEnumType;


typedef struct{
uint32_t readTimeout;
uint32_t writeTimeout;
}S1_INV;


extern int16_t _memoryMap[500];

/*!
 **************************************************************************************************
 *
 *  @fn         void S1_INV_registerToMbNode(void)
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
void S1_INV_registerToMbNode(void);
/*!
 **************************************************************************************************
 *
 *  @fn         void S1_INV_Database_Init()
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
void S1_INV_Database_Init();

/*!
 **************************************************************************************************
 *
 *  @fn         int  S1_INV_MbMng(void)
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
int  S1_INV_MbMng(void);
/*!
 **************************************************************************************************
 *
 *  @fn          void S1_INV_getInvUpdate(void)
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
 u16 S1_INV_getUpdateStatus(void);
 /*!
  **************************************************************************************************
  *
  *  @fn          void S1_INV_setInvUpdate(void)
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
  void S1_INV_setUpdateStatus(u16 u16Value);

#endif /* ASW_S1_INV_S1_INV_H_ */
