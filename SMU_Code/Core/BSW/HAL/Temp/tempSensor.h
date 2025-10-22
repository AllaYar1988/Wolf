/*
 * S_VGBR.h
 *
 *  Created on: Jan 20, 2025
 *      Author: 98912
 */

#ifndef _TEMPSENSOR_H_
#define _TEMPSENSOR_H_
#include "stdint.h"
   
 extern float _tempFinal, _tempref ;

   
 void readTempSensor(uint16_t sCalib);
 float getTemp(void);



#endif /* ASW_S5_VGBR_S5_VGBR_H_ */
