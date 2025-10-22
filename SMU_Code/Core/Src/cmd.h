/*
 * cmd.h
 *
 *  Created on: Oct 24, 2024
 *      Author: 98912
 */

#ifndef _CMD_H
#define _CMD_H

#include <stdio.h>
#include <string.h>
#include "Platform.h"

#define MAX_COMMANDS 20  // Maximum number of commands
#define MAX_CMD_LEN 50   // Maximum CMD LEN
// Define a type for command function pointers that accept a string argument
typedef u8 (*CommandFunc)(char *args);
typedef u8 (*helpFunc)(void);

typedef struct {
    char commandName[20];  // Command name
    CommandFunc execute;   // Function pointer to execute the command
    helpFunc help;
} Command;

// Declare command list and count as extern to be defined in the .c file
extern Command commandList[MAX_COMMANDS];
extern u16 commandCount;
extern u32 wcetArray[20];

// Function prototypes
u8 executeCommand( u8 *input);
void registerCommand(const char *name, CommandFunc function,helpFunc helpFunction) ;
// Example command functions
u8 debugCommand(  char *str);
u8 debugHelp( void);
u8 returnVersion( char *str);
u8 serialCommand(char *str);
u8 mcuControl( char *str);
u8 extract_serial_number(const char *input, char *output) ;
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
u8 helpCommand(char *str) ;

/*!
 **************************************************************************************************
 *
 *  @fn         u8 helpHelp(void);
 *
 *  @par        Help for Help
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
u8 helpHelp(void);

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
u8 mcuControlHelp( void);

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
u8 returnVersionHelp( void);
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
u8 wcetCommand(u8 *str);

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
u8 wcetCommandHelp(void);

#endif // CMD_H
