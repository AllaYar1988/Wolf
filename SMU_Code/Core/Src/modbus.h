/*
 * modbus.h
 *
 *  Created on: Oct 25, 2024
 *      Author: 98912
 */

#ifndef _MODBUS_H
#define _MODBUS_H
#include <stdint.h>

#define __ModuleReceive  6
#define __Modulesend     7
#define __TimeGSM        14

#define dmabufflen      2


#define SMU_ID        0x11
#define Inv_ID        0x11

#define __initialize          1
#define __idle                2
#define __ModbusSlaveReceive  3
#define __ModbusMasterReceive 4

#define __writeHMI           5
#define __readHMI            6
#define __Tower              7
#define __SIondelay          9
#define __SIoffdelay         10
#define __waittoTransmit     11
#define __SIOonRead          12
#define __SIOoffRead         13
#define __waittoRequest      14
#define __reqInv             15

#define __readReq             3
#define __writeReq           16

#define __SIO_Slave(x)  HAL_GPIO_WritePin (Slave_Select_GPIO_Port,Slave_Select_Pin,x);

typedef struct Modbus_HandleTypeDef {
    int  nDatabyte;                 // 4 bytes
    int  sAddress;                  // 4 bytes
    int  dataLen[2];                // 2 * 4 bytes = 8 bytes
    int byteCount;                  // 4 bytes
    char dstAddress;                // 1 byte
    char funCode;                   // 1 byte
    char Busy;                      // 1 byte
    char master;                    // 1 byte
    char f;                         // 1 byte
    char firstByte;                 // 1 byte
    char dataEnd;                   // 1 byte
    char receiveDataArray[256];     // 500 bytes
} Modbus_HandleTypeDef;
extern Modbus_HandleTypeDef _modbusMaster,_reqFrame;

typedef struct mbSlaveTypeDef {
    char  nDatabyte;                 // 4 bytes
    char  sAddress;                  // 4 bytes
    char byteCount;                  // 4 bytes
    char dstAddress;                // 1 byte
    char funCode;                   // 1 byte
    char Busy;                      // 1 byte
    char master;                    // 1 byte
    char firstByte;                 // 1 byte
    char dataEnd;                   // 1 byte
    char receiveDataArray[500];     // 500 bytes
} mbSlaveTypeDef;
extern mbSlaveTypeDef mbSlave;
extern int16_t _memoryMap[500];

int mbMasterEndOfFrameDetection(void);
int mbSlaveEndOfFrameDetection(void);
#endif /* SRC_MODBUS_H_ */
