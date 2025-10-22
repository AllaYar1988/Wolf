/*
* S3_HMI.c
*
*  Created on: Jan 19, 2025
*      Author: 98912
*/

#include "S3_HMI.h"
#include "sysvar.h"
#include "crc.h"
#include "S3_HMI.h"
#include "mac.h"
#include "RTE_MB.h"




S3_HMI s3Hmi;
MB_Slave_Struct s3Node;

int S3_HMI_sendWriteReq(void);



void S3_HMI_registerToMbNode(void){
  s3Node.id=S3_HMI_ID;
  s3Node.senReq=S3_HMI_MbMng;
  s3Node.resProcess=S3_HMI_resProcess;
  MB_Manager_RegisterSlave(s3Node);
  
}



int  S3_HMI_MbMng(void){
  static int state=READ;
  volatile int status=1;
  switch(state)
  {
  case WRITE:
    if(0==S3_HMI_sendWriteReq()){
      
      s3Hmi.writeTimeout=HAL_GetTick();
      state=WRITE_TIMEOUT;
    }
    break;
  case READ:
    if(0==S3_HMI_sendReadReq()){
      s3Hmi.readTimeout=HAL_GetTick();
      state=READ_TIMEOUT;
    }
    break;
  case WRITE_TIMEOUT:
    if((HAL_GetTick()-s3Hmi.writeTimeout)>S3_WRITE_TIMEOUT_VALUE){
      status=0;
      state=READ;
    }
    break;
  case READ_TIMEOUT:
    if((HAL_GetTick()-s3Hmi.readTimeout)>S3_READ_TIMEOUT_VALUE){
      state=WRITE;
    }
    break;
  }
  return status;
}

int S3_HMI_sendReadReq(void){
  static uint8_t mbStr[20];
  int status = 1;
  static int state=0;
  
  switch(state)
  {
  case 0:
    mbStr[0] = S3_HMI_ID;
    mbStr[1] = READ;
    mbStr[2] = (READ_START_ADDRESS & 0xff00) >> 8;
    mbStr[3] = READ_START_ADDRESS & 0x00ff;
    mbStr[4] = (READ_NUMBER_BYTE & 0xff00) >> 8;
    mbStr[5] = READ_NUMBER_BYTE & 0x00ff;
    uint16_t crc = calculateCRC(mbStr, 6); //
    mbStr[6] = crc & 0x00ff;
    mbStr[7] = (crc & 0xff00) >> 8;
    state=1;
    break;
  case 1:
    if(0==MAC_MbmSendData(mbStr,8)){
      state=0;
      status=0;
    }
    break;
  }
  return status;
}

int S3_HMI_sendWriteReq(void){
  static uint8_t mbStr[S3_WRITE_BUFFER_SIZE];
  int status = 1;
  static int kindex = 7;
  static int state=0;
  
  switch(state)
  {
  case 0:
    mbStr[0]= S3_HMI_ID;
    mbStr[1] = WRITE;
    mbStr[2] = (WRITE_START_ADDRESS & 0xff00) >> 8;
    mbStr[3] =  WRITE_START_ADDRESS & 0x00ff;
    mbStr[4] = (WRITE_NUMBER_BYTE & 0xff00) >> 8;
    mbStr[5] =  WRITE_NUMBER_BYTE & 0x00ff;
    mbStr[6] = 2 * WRITE_NUMBER_BYTE;
    kindex = 7;
    for (int index = 0; index < WRITE_NUMBER_BYTE; index++) {
      mbStr[kindex++] = (((int) (_memoryMap[index
                                 + WRITE_START_ADDRESS])) & 0xff00) >> 8;
      mbStr[kindex++] = ((int)  (_memoryMap[index
                                 + WRITE_START_ADDRESS])) & 0x00ff;
      
    }
    uint16_t crc = calculateCRC(mbStr, kindex); //
    mbStr[kindex++] = crc & 0x00ff;
    mbStr[kindex++] = (crc & 0xff00) >> 8;
    state=1;
    break;
  case 1:
    if(0==MAC_MbmSendData(mbStr,kindex)){
      state=0;
      status=0;
      memset(mbStr,0,S3_WRITE_BUFFER_SIZE);
    }
    break;
  }
  return status;
  
}




void S3_HMI_resProcess(char *res,int Len)
{
  int ind = 0;
  int nDb = 0;
  int dummyHMI[200];
  if (checkFrameCRC(res, Len)) {
    switch (res[1]) {
    case 0x03:
      _memoryMap[200] = ~_memoryMap[200];
      nDb = res[2]; 
      
      for (int index = 0; index < nDb; index++) {
        ind = 2 * index + 3;
        dummyHMI[index] = (res[ind] << 8
                           | res[ind + 1]);
      }
      //char pVoidVar = (char) dummyHMI[38];

        for (int index = 0; index <= 40; index++) {
          _memoryMap[index] = dummyHMI[index];
        }
      
      
      break;
    case 0x10:
      
      break;
    default:
      break;
    }
  }
}





