/*
 * S_VGBR.h
 *
 *  Created on: Jan 20, 2025
 *      Author: 98912
 */

#ifndef ASW_S5_VGBR_S5_VGBR_H_
#define ASW_S5_VGBR_S5_VGBR_H_
#include "main.h"
#include "modbus.h"
#include "string.h"

#define S5_VGBR_ID 0x12
#define S5_READ_START_ADDRESS  0
#define S5_READ_NUMBER_BYTE   40
#define S5_WRITE_START_ADDRESS  100
#define S5_WRITE_NUMBER_BYTE   40
/*State*/
#define READ_TIMEOUT  100
#define WRITE_TIMEOUT 101
/*Value*/
#define S5_WRITE_TIMEOUT_VALUE 200
#define S5_READ_TIMEOUT_VALUE  200

#define S5_WRITE_BUFFER_SIZE 256


typedef struct{
uint32_t readTimeout;
uint32_t writeTimeout;
}S5_VGBR;

extern int   _countVgB ;

extern float _memoryMap_VgBF[100];
extern int16_t _memoryMap[500];
int S5_VGBR_MbMng(void);
void S5_VGBR_resProcess(char *res,int Len);
int S5_VGBR_sendReadReq();
int S5_VGBR_sendWriteReq();
void S5_VGBR_registerToMbNode(void);
#endif /* ASW_S5_VGBR_S5_VGBR_H_ */
