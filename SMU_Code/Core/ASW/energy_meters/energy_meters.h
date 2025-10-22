/*
 * energy_meters.h
 *
 *  Created on: Oct 16, 2025
 *      Author: KHAJEHHOD-GD09
 */

#ifndef ASW_ENERGY_METERS_ENERGY_METERS_H_
#define ASW_ENERGY_METERS_ENERGY_METERS_H_

#define ENERGY_METER_TIMEOUT 100

typedef enum {
	ENU_EM_INIT=0,
	ENU_EM_SEND_READ_REQ,
	ENU_EM_WAIT_FOR_RESPONSE,
	ENU_EM_STOP
}EnuEnrgyMeterState;

void energy_meters_handler(void);

#endif /* ASW_ENERGY_METERS_ENERGY_METERS_H_ */
