#include "S5_VGBR.h"
#include "crc.h"
#include "S3_HMI.h"
#include "Ctrl.h"
#include "mntdata.h"
#include "mac.h"
#include "RTE_MB.h"


S5_VGBR s5Vgbr;
float _memoryMap_VgBF[100];
int   _countVgB = 0;

MB_Slave_Struct s5Node;

void S5_VGBR_registerToMbNode(void){
  s5Node.id=S5_VGBR_ID;
  s5Node.senReq=S5_VGBR_MbMng;
  s5Node.resProcess=S5_VGBR_resProcess;
  MB_Manager_RegisterSlave(s5Node);
  
}


int  S5_VGBR_MbMng(void){
  static int state=READ;
  volatile int status=1;
  switch(state)
  {
  case WRITE:
    if(0==S5_VGBR_sendWriteReq()){
      
      s5Vgbr.writeTimeout=HAL_GetTick();
      state=WRITE_TIMEOUT;
    }
    break;
  case READ:
    if(0==S5_VGBR_sendReadReq()){
      s5Vgbr.readTimeout=HAL_GetTick();
      state=READ_TIMEOUT;
    }
    break;
  case WRITE_TIMEOUT:
    if((HAL_GetTick()-s5Vgbr.writeTimeout)>S5_WRITE_TIMEOUT_VALUE){
      status=0;
      state=READ;
    }
    break;
  case READ_TIMEOUT:
    if((HAL_GetTick()-s5Vgbr.readTimeout)>S5_READ_TIMEOUT_VALUE){
      state=WRITE;
    }
    break;
  }
  return status;
}
void S5_VGBR_resProcess(char *res,int Len)
{ uint32_t vgbrData=0;    
int nDb = 0;
int ind = 0;
if (checkFrameCRC(res, Len)) {
  switch (res[1]) {
  case 0x03:
    _countVgB = 0;
    _memoryMap[250] = (~_memoryMap[250])&0x01;

      _memoryMap[250] = 0;
    nDb = res[2];
    for (int index = 0; index < nDb/4; index++) {
      ind = 4 * index + 3;
      vgbrData = (res[ind]
                  << 24 | res[ind + 1] << 16
                    | res[ind + 2] << 8
                      | res[ind + 3]);
      memcpy(&_memoryMap_VgBF[index], &vgbrData,
             sizeof(float));
    }
    _memoryMap[251] = (int) (_memoryMap_VgBF[0] * 10);
    	_memoryMap[252] = (int) (_memoryMap_VgBF[1] * 10);
    	_memoryMap[253] = (int) (_memoryMap_VgBF[2] * 10);
    break;
  case 0x10:
    
    break;
  default:
    break;
  }
}
}


int S5_VGBR_sendReadReq(void){
  static uint8_t mbStr[20];
  int status = 1;
  static int state=0;
  uint16_t crc =0;
  
  switch(state)
  {
  case 0:
    mbStr[0] = S5_VGBR_ID;
    mbStr[1] = READ;
    mbStr[2] = (S5_READ_START_ADDRESS & 0xff00) >> 8;
    mbStr[3] = S5_READ_START_ADDRESS & 0x00ff;
    mbStr[4] = (S5_READ_NUMBER_BYTE & 0xff00) >> 8;
    mbStr[5] = S5_READ_NUMBER_BYTE & 0x00ff;
    crc = calculateCRC(mbStr, 6); //
    mbStr[6] = crc & 0x00ff;
    mbStr[7] = (crc & 0xff00) >> 8;
    state=1;
    break;
  case 1:
    if(0==MAC_MbmSendData((uint8_t*)mbStr,8)){
      state=0;
      status=0;
    }
    break;
  }
  return status;
}

int S5_VGBR_sendWriteReq(void){
  static uint8_t mbStr[S5_WRITE_BUFFER_SIZE];
  int status = 1;
  static u16 kindex = 7;
  float dummyfloat = 0;
  unsigned long int lintff = 0;
  uint16_t crc =0;
  
  //Database_Type *mDb = getMntDatabase();
  
  static int state=0;
  
  switch(state)
  {
  case 0:
    mbStr[0]= S5_VGBR_ID;
    mbStr[1] = WRITE;
    mbStr[2] = (S5_WRITE_START_ADDRESS & 0xff00) >> 8;
    mbStr[3] =  S5_WRITE_START_ADDRESS & 0x00ff;
    mbStr[4] = (S5_WRITE_NUMBER_BYTE & 0xff00) >> 8;
    mbStr[5] =  S5_WRITE_NUMBER_BYTE & 0x00ff;
    mbStr[6] = 2 * S5_WRITE_NUMBER_BYTE;
    
    kindex = 7;
    for (int index = 0; index < S5_WRITE_NUMBER_BYTE/2; index++) {
      if (index != 0)
        dummyfloat = (float) _memoryMap[index] / 10.0;
      else if ((index == 8) || (index == 9) || (index == 10))
        dummyfloat = _memoryMap_VgBF[index - 8];
      else
        dummyfloat = (float) _memoryMap[index];
      memcpy(&lintff, &dummyfloat, sizeof(float));
      
      mbStr[kindex++] = ((lintff) & 0xff00) >> 8;
      mbStr[kindex++] = (lintff) & 0x00ff;
      mbStr[kindex++] = ((lintff) & 0xff000000) >> 24;
      mbStr[kindex++] = ((lintff) & 0xff0000) >> 16;
    }

     crc = calculateCRC(mbStr, kindex); //
    mbStr[kindex++] = crc & 0x00ff;
    mbStr[kindex++] = (crc & 0xff00) >> 8;
    state=1;
    
    break;
  case 1:
    if(0==MAC_MbmSendData(mbStr,kindex)){
      state=0;
      status=0;
      memset(mbStr,0,100);
    }
    break;
  }
  return status;
  
}
