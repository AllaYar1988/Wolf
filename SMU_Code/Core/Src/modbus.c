/*
 * modbus.c
 *
 *  Created on: Oct 25, 2024
 *      Author: 98912
 */
#include "modbus.h"

Modbus_HandleTypeDef _modbusMaster,_reqFrame;
mbSlaveTypeDef  mbSlave;
int16_t _memoryMap[500];

int mbSlaveEndOfFrameDetection(void)
{
	int rt=0;
	if(1==mbSlave.firstByte){
		if (mbSlave.dataEnd++ > 4) //2ms
		{
			mbSlave.dataEnd = 0;
			mbSlave.firstByte = 0;
			mbSlave.Busy = 1;
			//_stateArray[9] = __ModbusSlaveReceive;
			rt=3;
		}
	}
	else
	{
		mbSlave.dataEnd=0;
	}

return rt;
}

int mbMasterEndOfFrameDetection(void)
{
	int rt=0;
	if(1==_modbusMaster.firstByte){
		if (_modbusMaster.dataEnd++ > 4) //2ms
		{
			_modbusMaster.dataEnd = 0;
			_modbusMaster.firstByte = 0;
			_modbusMaster.Busy = 1;
			rt=4;
		}
	}
	else
	{
		_modbusMaster.dataEnd=0;
	}

return rt;
}
