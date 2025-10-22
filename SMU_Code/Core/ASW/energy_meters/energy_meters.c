/*
 * energy_meters.c
 *
 *  Created on: Oct 16, 2025
 *      Author: KHAJEHHOD-GD09
 */
#include "energy_meters.h"
#include "crc.h"
#include "energy_meter_hal.h"
#include "energy_meter_dll.h"

static u8 energy_meters_send_read_req(u8 addr);
static u8 energy_meters_process(void);
static u32 gEnergyMeterTimeout=0;


void energy_meters_handler(void)
{
	static EnuEnrgyMeterState state=ENU_EM_INIT;

	switch(state)
	{
	case ENU_EM_INIT:
		energy_meters_hal_init();
		state=ENU_EM_SEND_READ_REQ;
		break;
	case ENU_EM_SEND_READ_REQ:
		//ENER_GEN_CS(GPIO_PIN_SET)
		HAL_GPIO_WritePin(uGenerator_STPM_SCS_GPIO_Port, uGenerator_STPM_SCS_Pin, GPIO_PIN_SET);

		energy_meters_send_read_req(0x05);
		state=ENU_EM_WAIT_FOR_RESPONSE;
		break;

	case ENU_EM_WAIT_FOR_RESPONSE:
		//ENER_GEN_CS(GPIO_PIN_RESET)
		HAL_GPIO_WritePin(uGenerator_STPM_SCS_GPIO_Port, uGenerator_STPM_SCS_Pin, GPIO_PIN_RESET);
		u8 temp=energy_meters_process();
		if(temp==1)
		{
			state=ENU_EM_SEND_READ_REQ;
		}
		break;
	case ENU_EM_STOP:
		break;
	}
}



static u8 energy_meters_send_read_req(u8 addr)
{
	u8 returnValue=0;
	u8 txBuf[20];
    txBuf[0] = addr | 0x80; // read bit
    txBuf[1] = txBuf[2] = txBuf[3] = txBuf[4] = 0xFF;
     //(void) addr;

    //uint8_t crc = crc_stmp3x(txBuf, 4);
    //if (crc != rxBuf[4])
    //{
     //   printf("CRC error!\r\n");
     //   return 0;
    //}

    //uint32_t val = ((uint32_t)rxBuf[1] << 16) | ((uint32_t)rxBuf[2] << 8) | rxBuf[3];
    //return val;
     energy_meter_dll_send(txBuf,5);
     return returnValue;
}

static u8 energy_meters_process(void)
{
	u8 returnValue=0;
	if(gEnergyMeterTimeout++>ENERGY_METER_TIMEOUT)
	{
		gEnergyMeterTimeout=0;
		returnValue=1;
	}

	return returnValue;
}
