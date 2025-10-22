/*
 * sysvar.c
 *
 *  Created on: Oct 25, 2024
 *      Author: 98912
 */

#include "sysvar.h"
#include "Platform.h"

// Static variable to hold the singleton instance
static SysVarStruct *mySysVar = NULL;
int dbgCtrl=0;
// Function to get the singleton instance
SysVarStruct* getSysvarInstance() {
	if (mySysVar == NULL) {
		mySysVar = (SysVarStruct*) malloc(sizeof(SysVarStruct));
		if (mySysVar == NULL) {
			// Handle malloc failure
			return NULL;
		}
	}
	return mySysVar;
}

// Function to set a variable in the singleton instance using offset
void setSysVar(SYS_EnuSysVarIdType enuSysVarID, const void *pVoidVar,
		size_t size) {
	SysVarStruct *SysVarL = getSysvarInstance();
	if (SysVarL != NULL && pVoidVar != NULL) {
		void *destination = (char*) SysVarL + enuSysVarID;
		memcpy(destination, pVoidVar, size);
	}
}

// Function to get a variable from the singleton instance using offset
void getSysVar(SYS_EnuSysVarIdType enuSysVarID, void *pVoidVar, size_t size) {
	SysVarStruct *SysVarL = getSysvarInstance();
	if (SysVarL != NULL && pVoidVar != NULL) {
		void *source = (char*) SysVarL + enuSysVarID;
		memcpy(pVoidVar, source, size);
	}
}

void setMainEventQueueIndex(MAIN_STATE INDEX, MAIN_STATE value)
{
	(void)value;/* To avoid unused warning */
	(void)INDEX;/* To avoid unused warning */
	//if (INDEX < 0 || INDEX >= MS_NUM_STATE) {
		// Handle out-of-bounds index
	//	return;
   //	}

	// Calculate the offset for the specific index in the mainEventQueue
	//size_t offset = SYS_MAIN_EVENT_QUEUE + INDEX * sizeof(MAIN_STATE);

	// Set the specific value at the calculated offset
	//setSysVar(offset, &value, sizeof(MAIN_STATE));
}

void initSysVar() {
	char ptr=0;
setSysVar(SYS_DBG_EN,&ptr,SIZE_CH);
setSysVar(SYS_GPRS_RESET,&ptr,SIZE_CH);
setSysVar(SYS_GPRS_TIME_OUT,&ptr,SIZE_CH);
setSysVar(SYS_MODBUS_RESET,&ptr,SIZE_CH);
setSysVar(SYS_MODBUS_TIME_OUT,&ptr,SIZE_CH);
setSysVar(SYS_REMOTE_LOCAL,&ptr,SIZE_CH);
}

