/*
 * S6_WEB.c
 *
 *  Created on: Feb 21, 2025
 *      Author: 98912
 */
/*
 * S3_HMI.c
 *
 *  Created on: Jan 19, 2025
 *      Author: 98912
 */

#include "S6_WEB.h"
#include "crc.h"
#include "mac.h"
#include "RTE_MB.h"
#include "mntdata.h"


S6_WEB s6Web;
MB_Slave_Struct s6Node;
#define S6_WEB_ID 0x00
#define READ_START_ADDRESS  0
#define READ_NUMBER_BYTE   64
#define WRITE_START_ADDRESS  900
#define WRITE_NUMBER_BYTE   60

/*State*/
#define WRITE 16
#define READ 3
#define READ_TIMEOUT  100
#define WRITE_TIMEOUT 101
/*Value*/
#define S6_WRITE_TIMEOUT_VALUE 100
#define S6_READ_TIMEOUT_VALUE  100

#define S6_WRITE_BUFFER_SIZE 256




int S6_WEB_sendWriteReq(void);
int S6_WEB_MbMng(void);
void S6_WEB_resProcess(char *res,int Len);

void S6_WEB_registerToMbNode(void){
	s6Node.id=0x00;
	s6Node.senReq=S6_WEB_MbMng;
	s6Node.resProcess=S6_WEB_resProcess;
	MB_Manager_RegisterSlave(s6Node);

}



int  S6_WEB_MbMng(void){
	static int state=WRITE;
	volatile int status=1;
	switch(state)
	{
	case WRITE:
		if(0==S6_WEB_sendWriteReq()){

			s6Web.writeTimeout=HAL_GetTick();
			state=WRITE_TIMEOUT;
		}
		break;
	case WRITE_TIMEOUT:
		if((HAL_GetTick()-s6Web.writeTimeout)>S6_WRITE_TIMEOUT_VALUE){
			status=0;
			state=WRITE;
		}
		break;
		break;
	}
	return status;
}



int S6_WEB_sendWriteReq(void){
	static uint8_t mbStr[S6_WRITE_BUFFER_SIZE];
	int status = 1;
	RefDataType *rData=getRefData();
	static u16 kindex = 7;
	static int state=0;

	switch(state)
	{
	case 0:
		mbStr[0]= 0x02;
		mbStr[1] = WRITE;
		mbStr[2] = (WRITE_START_ADDRESS & 0xff00) >> 8;
		mbStr[3] =  WRITE_START_ADDRESS & 0x00ff;

		kindex = 7;
		int index=0;
		while(1)
		{
			u16 u16RefId=(u16) getRefIDValue();

			if(0==u16RefId)
				{

				state=0;
				status=0;
				memset(mbStr,0,100);
				break;
			}
			else
			{
				int dummy=0;
				if(strcmp(rData[index].name,"REFID")==0)
				{
					dummy=(int)(rData[index].value);
				}
				else //if(REF_UPDATED_VALUE == (rData[index].flag & REF_UPDATED_VALUE))
				{
				 dummy=(int)(rData[index].value*10);
				}
				mbStr[kindex++]=(dummy & 0xff00) >> 8;
				mbStr[kindex++]=dummy  & 0x00ff;
				if(0==strcmp(rData[index].name,"RefID") || index>=(REF_ARRAY_SIZE-1)){
					index++;
					break;
				}
				index++;
			}
		}
		if(1==status){
			mbStr[4] = (index & 0xff00) >> 8;
					mbStr[5] =  index & 0x00ff;
					mbStr[6] = 2 * index;
		uint16_t crc = calculateCRC(mbStr, kindex); //
		mbStr[kindex++] = crc & 0x00ff;
		mbStr[kindex++] = (crc & 0xff00) >> 8;
		state=1;
		}
		break;
	case 1:
		if(0==MAC_MbmSendData(mbStr,kindex)){
			state=0;
			status=0;
			memset(mbStr,0,256);
		}
		break;
	}
	return status;

}




void S6_WEB_resProcess(char *res,int Len)
{
(void)Len;
(void)res;
}







