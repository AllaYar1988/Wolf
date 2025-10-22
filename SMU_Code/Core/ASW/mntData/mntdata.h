/*
 * mntdata.h
 *
 *  Created on: Jan 14, 2025
 *      Author: kh.166
 */

#ifndef MNTDATA_H_
#define MNTDATA_H_
#include <stdint.h>
#include <stddef.h>
#include "Platform.h"

#define MONITORING_ARRAY_SIZE 50
#define REF_ARRAY_SIZE 30

#define REF_SIZE 10
#define DATABASE_SIZE 10
#define LOCAL 1
#define REMOTE 0

#define REF_REPORT_TO_WEB  0x08
#define REF_WRITE_TO_MEM   0x02
#define REF_UPDATED_VALUE  0x01

typedef enum{
	INV_WRITE_EN_INDEX=0,
	INV_SYS_MOD_INDEX=1,
	INV_PREF_GC_INDEX=2,
	INV_WC_PREF_GC_INDEX=3,
	INV_Q_STAR_INDEX=4,
	INV_Q_UP_LIMIT_INDEX=5,
	INV_Q_LOW_LIMIT_INDEX=6,
	INV_V_BATT_INDEX=7,
	INV_EX_VM1_INDEX=8,
	INV_EX_VM2_INDEX=9,
	INV_EX_WPLL_INDEX=10,
	INV_V_AMP_UP_LIMIT_INDEX=11,
	INV_V_AMP_LOW_LIMIT_INDEX=12,
	INV_VDC_START_INDEX=13,
	INV_VDC_STOP_INDEX=14,
	INV_VBATT_CH_INDEX=15,
	INV_I_TH_INDEX=16,
	INV_RMS_OVER_TIME_INDEX=17,
	INV_ITH_RMS_INDEX=18,
	INV_LOAD_RLY_INDEX=19,
	INV_SERVER_INDEX=20,
	CH_MOD_WRITE_EN_INDEX=100,
	CH_PWR_ON_USER_CMD_INDEX=101,
	CH_LOW_BATT_LOW_TH_INDEX=102,
	CH_LOW_BATT_HI_TH_INDEX=103,
	CH_PWR_LOW_CUTTOFF_INDEX=104,
	CH_LOW_BATT_F_LOW_TH_INDEX=105,
	CH_LOW_BATT_F_HI_TH_INDEX=106,
	REFID_INDEX=32000

}REF_WEB_INDEX;





typedef struct {
	char  			ref[REF_SIZE];
	char  			name[NAME_SIZE];
	char  			unit[UNIT_SIZE];
	float 			value;
	REF_WEB_INDEX 	index;
	char 			flag;
}RefDataType;

typedef struct
{
    const char  *mName;
    MntDataType *mData;
    size_t       mDataSize;
} Database_Type;



//MntDataType* getMntData(void);
int registerToDatabase(const char *moduleName,
                      MntDataType *moduleDb,
                      size_t moduleDatasize);
  Database_Type* getMntDatabase(void);
  int getSizeOfRgsModule(void);
  RefDataType* getRefData(void);
  float getRefIDValue(void) ;
  void setRefIDValue(float x) ;
  uint16_t getRefIDIndex() ;
  u8 refSetCommand( char *str);
  u8 refSetCommandHelp( void );
  /*!
   **************************************************************************************************
   *
   *  @fn         void setSendCfg( u8 )
   *
   *  @par        Help for This function is for Set Ref. Values.
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
  void setSendCfg( u8 value );
  /*!
   **************************************************************************************************
   *
   *  @fn         u8 getSendCfg( void )
   *
   *  @par        Help for This function is for Set Ref. Values.
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
  u8  getSendCfg( void);

#endif /* MNTDATA_H_ */
