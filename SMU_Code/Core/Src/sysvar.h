/*
 * sysvar.h
 *
 *  Created on: Oct 25, 2024
 *      Author: 98912
 */

#ifndef _SYSVAR_H
#define _SYSVAR_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>  // For offsetof

#define SIZE_CH sizeof(char)
#define SIZE_INT sizeof(int)
#define BIT_POS_MASK                0xE
#define BYTE_ADDR_MASK              0xFFF0
#define BYTE_LEN_MASK               BIT_POS_MASK
#define BUFFER_ID_BIT_POS_MASK      0x7
#define BUFFER_ID_BYTE_ADDR_MASK    0x38
#define BUFFER_ID_SIG_SIZE_MASK     0xFC0
#define BUFFER_ID_FRAME_INDEX_MASK  0xF000
#define NO_VALID_INFO               0xFFFF
#define SYSVAR_PROFILE_ARRAY_SIZE   ((u8)(4))

#define SYS_VAR_INFO(byte_addr, pos_len, flag) (((byte_addr) << 4) | ((pos_len) << 1) | (flag))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef enum {
    MS_NONE = 0,
    MS_ACTIVE,
    MS_INITIALIZE,
    MS_IDLE,
    MS_RTC,
    MS_PERIODIC_1SEC,
    MS_MODBUS_RECEIVED,
    MS_RETURN_CONFIG,
    MS_WRITE_DATA,
    MS_WRITE_DIGITAL_DATA,
    MS_SAVE_CALIB,
    MS_NUM_STATE
} MAIN_STATE;

#include <stddef.h> // For size_t

// Assume a packed structure with no padding
typedef struct {
    char digitalInputEnable;      // Offset: 0
    char modbusSlaveId;           // Offset: 1
    char modbusReset;             // Offset: 2
    char gprsReset;               // Offset: 3
    char modbusTimeout;           // Offset: 4
    char gprsTimeOut;             // Offset: 5
    char _remoteLocal;            // Offset: 6
    int mainEventQueue[10];       // Offset: 8 (aligned to 4 bytes)
    int dbgTime;                  // Offset: 48
    char dbgEN;                   // Offset: 52
    float calibration[12];        // Offset: 56
} SysVarStruct;

// Enum with constant offsets
typedef enum {
    SYS_DIGITAL_INPUT_ENABLE   = 0,
    SYS_MODBUS_SLAVE_ID        = 1,
    SYS_MODBUS_RESET           = 2,
    SYS_GPRS_RESET             = 3,
    SYS_MODBUS_TIME_OUT        = 4,
    SYS_GPRS_TIME_OUT          = 5,
    SYS_REMOTE_LOCAL           = 6,
    SYS_MAIN_EVENT_QUEUE       = 8,
    SYS_DBG_TIME               = 48,
    SYS_DBG_EN                 = 52,
    SYS_CALIBRATION            = 56
} SYS_EnuSysVarIdType;

extern int dbgCtrl;
// Function declarations
void initSysVar();
void setSysVar(SYS_EnuSysVarIdType enuSysVarID, const void *pVoidVar, size_t size);
void getSysVar(SYS_EnuSysVarIdType enuSysVarID, void *pVoidVar, size_t size);
SysVarStruct* getSysvarInstance();
void setMainEventQueueIndex(MAIN_STATE index, MAIN_STATE value);

#endif // __SYSVAR_H


