/*
 * cmd.c
 *
 *  Created on: Oct 24, 2024
 *      Author: 98912
 */
#include "cmd.h"
#include "sysvar.h"
#include "dbg.h"

Command commandList[MAX_COMMANDS];
u16 commandCount = 0;

/*!
 **************************************************************************************************
 *
 *  @fn         void registerCommand(const char *name, CommandFunc function,helpFunc helpFunction) {
 *
 *  @par        This function is for Register Commands into Command list.
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
void registerCommand(const char *name, CommandFunc function,helpFunc helpFunction) {
	if (commandCount < MAX_COMMANDS) {
		strncpy(commandList[commandCount].commandName, name, sizeof(commandList[commandCount].commandName) - 1);
		commandList[commandCount].commandName[sizeof(commandList[commandCount].commandName) - 1] = '\0'; // Ensure null termination
		commandList[commandCount].execute = function;
		commandList[commandCount].help = helpFunction;
		commandCount++;
	} else {
		// sendSerial("Command list is full!\n");
	}
}
/*!
 **************************************************************************************************
 *
 *  @fn         u8 executeCommand( u8 *input)
 *
 *  @par        This function is for execute Commands from Command list.
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
u8 executeCommand( u8 *input) {
	static u8 state=0;
	static u8 cmdIndex=0;
	static u8 dataStr[MAX_CMD_LEN];
	u8 returnValue=1;
	switch(state)
	{
	case 0:
		for (int i = 0; i < commandCount; i++) {
			if (strstr((char*)input,(char*) commandList[i].commandName) == (char*)input)
			{  // Check if the input starts with commandName
				//char *args = input ;//+ strlen(commandList[i].commandName);  // Get the tail (args) after the command name
				//while (*args == ' ') args++;  // Skip any leading spaces
				// Pass the tail to the execute function
				memcpy(dataStr,input,MAX_CMD_LEN);
				cmdIndex=i;
				state=1;

			}
		}
		if(0==state)
		{
			returnValue=0;
		}
		break;
	case 1:
		if(0==commandList[cmdIndex].execute((char *)dataStr))
		{
			returnValue=0;
			state=0;
		}
		break;
	}

	// sprintf((char *)input,"Unknown command\n");
	return returnValue;
}
/*!
 **************************************************************************************************
 *
 *  @fn         u8 debugCommand( char *str)
 *
 *  @par        This function is for handling debug commands.
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
u8 debugCommand( char *str)
{
	u8 returnVlaue=1;

	// Check if the command is "on"
	if (strstr((char *)str, "on")) {
		TransmitCMDResponse("Debug is ON\r");
		dbgCtrl=1;
		returnVlaue=0;
	}
	// Check if the command is "off"
	else if (strstr((char *)str, "off")) {
		TransmitCMDResponse("Debug is OFF\r");
		dbgCtrl=0;
		returnVlaue=0;
	}
	return returnVlaue;
}
/*!
 **************************************************************************************************
 *
 *  @fn         u8 debugHelp( void)
 *
 *  @par        This function is for handling debug commands help.
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
u8 debugHelp( void)
{
	static u8 state=0;
	u8 returnVlaue=1;
	switch(state)
	{
	case 0:
		TransmitCMDResponse("     dbg on                 -> (Enables debug port for 60 minute) \r");
		state=1;
		break;
	case 1:
		TransmitCMDResponse("     dbg off                -> (Disables Debug) \r");

		state=0;
		returnVlaue=0;
		break;
	}
	return returnVlaue;
}

/*!
 **************************************************************************************************
 *
 *  @fn         u8 returnVersion( char *str)
 *
 *  @par        Function to return the software version
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
u8 returnVersion( char *str)
{
	int len=sizeof(str);
	memset(str,0,len);
	TransmitCMDResponse(SW_VERSION);
	return 0;
}
/*!
 **************************************************************************************************
 *
 *  @fn         u8 returnVersionHelp( void)
 *
 *  @par        Help for Function to return the software version
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
u8 returnVersionHelp( void)
{
	TransmitCMDResponse("     version                -> (Returns Software Version) \r");
	return 0;
}
/*!
 **************************************************************************************************
 *
 *  @fn        u8 mcuControl( char *str)
 *
 *  @par        Function to control MCU
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
u8 mcuControl( char *str)
{
	if (strstr((char *)str, "reset")) {

		while(1){};
	}
return 0;
}
/*!
 **************************************************************************************************
 *
 *  @fn         u8 mcuControlHelp( void)
 *
 *  @par        Help Function to Control MCU
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
u8 mcuControlHelp( void)
{
	TransmitCMDResponse("     mcu reset              -> (To reset mcu \r");
	return 0;

}
/*!
 **************************************************************************************************
 *
 *  @fn         u8 returnVersion( char *str)
 *
 *  @par        Function to return the software version
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
u8 helpCommand(char *str) {
	static u8 state=0;
	static u8 nextState=0;
	static u16 wait=0;
	static u16 cmdIndex=0;
	u8 returnVlaue=1;
   (void)str; /* To avoid unused warning */

	switch (state){
	case 0:
		TransmitCMDResponse("This device has following commands:\r");
		state=255;
		nextState=1;
		break;
	case 1:

		if(0==commandList[cmdIndex].help())
		{
			cmdIndex++;
			state=255;
			nextState=1;
		}
		else
		{
			state=255;
			nextState=1;
		}
		if(cmdIndex>=commandCount)
		{
			state=0;
			nextState=0;
			returnVlaue=0;
			cmdIndex=0;
		}
		break;
	case 255:
		if(++wait>100)
		{
			wait=0;
			state=nextState;
		}
		break;

	}
	return returnVlaue;
}

/*!
 **************************************************************************************************
 *
 *  @fn         u8 returnVersion( char *str)
 *
 *  @par        Function to return the software version
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
u8 helpHelp(void)
{
	TransmitCMDResponse("     help                   -> (Shows available commands on this device) \r");
	return 0;
}
/*!
 **************************************************************************************************
 *
 *  @fn         u8 wcetCommand(u8 *str)
 *
 *  @par        Function for WCET
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
u8 wcetCommand(u8 *str)
{
	(void)str;/* To avoid unused warning */
	TransmitCMDResponse("Reset WCET Array\r");
	memset(wcetArray,0,19);
	return 0;
}
/*!
 **************************************************************************************************
 *
 *  @fn         u8 wcetCommandHelp(void)
 *
 *  @par        Function for WCET Handle
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
u8 wcetCommandHelp(void)
{
	TransmitCMDResponse("     WCET reset             -> (To Reset WCET Holding Array) \r");
	return 0;
}


