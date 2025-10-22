/*
 * S3_HMI.h
 *
 *  Created on: Jan 19, 2025
 *      Author: 98912
 */

#ifndef ASW_S3_HMI_S3_HMI_H_
#define ASW_S3_HMI_S3_HMI_H_
#include "main.h"
#include "string.h"
#include "modbus.h"

#define S3_HMI_ID 0x02
#define READ_START_ADDRESS  0
#define READ_NUMBER_BYTE   64
#define WRITE_START_ADDRESS  200
#define WRITE_NUMBER_BYTE   60

/*State*/
#define WRITE 16
#define READ 3
#define READ_TIMEOUT  100
#define WRITE_TIMEOUT 101
/*Value*/
#define S3_WRITE_TIMEOUT_VALUE 200
#define S3_READ_TIMEOUT_VALUE  200

#define S3_WRITE_BUFFER_SIZE 256


typedef struct{
uint32_t readTimeout;
uint32_t writeTimeout;
}S3_HMI;

extern int16_t _memoryMap[500];

int S3_HMI_MbMng(void);
void S3_HMI_resProcess(char *res,int Len);
int S3_HMI_sendReadReq(void);
int S3_HMI_sendWriteReq(void);
void S3_HMI_registerToMbNode(void);
#endif /* ASW_S3_HMI_S3_HMI_H_ */
