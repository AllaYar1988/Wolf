                                      /*******************
 ***************************************** C SOURCE FILE ******************************************
                                        *******************/
/**
 *  @file       iflash.C/.H
 *
 *  General Info
 *  ------------
 *  ____
 *  @par        File info
 *  @li @b      Version         : 1.0.4
 *  @li @b      Date            : 2024-05-14
 *
 *  @par        Project info
 *  @li @b      Project         : xxxxx
 *  @li @b      Processor       : STM32f4xx
 *  @li @b      Tool  @b Chain  :
 *  @li @b      Clock @b Freq   : 168 MHz
 *
 *  @par        This file create for use Media Access Control (MAC) for UART
 *
 *  @copyright  (C)
 *
 **************************************************************************************************
 *  _______________
 *  Version History
 *  ---------------
  **************************************************************************************************
 *
 *              This veraion has been tested.
 *  ____
 *  @par        Rev 1.0.0
 *  @li @b      Date            : 2025-05-13
 *  @li @b      Author          : A.Moazami
 *  @li @b      Approved @b by  :
 *  @li @b      Description
 *
 *              Initial Version
 *
 **************************************************************************************************
 */
#ifndef _IFLASH_H
#define _IFLASH_H
#include "Platform.h"




#define FLASH_USER_START_ADDR   0x080E0000   // Start address of sector 6
#define FLASH_USER_END_ADDR     (0x080E0000 + 0x1000)  // Define a range, e.g., 4KB
extern char _serialN[10];


void Flash_Write_String(uint32_t address, const char *data);
void Flash_Read_String(uint32_t address, char *buffer, uint32_t max_length);
u8 extract_serial_number(const char *input, char *output) ;
/*!
 **************************************************************************************************
 *
 *  @fn         u8 serialCommand(char *str)
 *
 *  @par        This function is for Set Set/Get/Save Serial Number.
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
u8 serialCommand(char *str);

/*!
 **************************************************************************************************
 *
 *  @fn         u8 serialCommandHelp()
 *
 *  @par        This function is Help for Set Set/Get/Save Serial Number.
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
u8 serialCommandHelp(void);

#endif // IFLASH_H
