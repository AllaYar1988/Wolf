                                      /*******************
 ***************************************** C SOURCE FILE ******************************************
                                        *******************/
/**
 *  @file       iFlash.C/.H
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

#include "iflash.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash.h"
#include "dbg.h"

 char _serialN[10]="UA001";


void Flash_Write_String(uint32_t address, const char *data) {
    HAL_FLASH_Unlock();

    // Clear all pending flags
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    // Erase the sector before writing (only needed once if writing multiple strings)
    FLASH_Erase_Sector(FLASH_SECTOR_11, VOLTAGE_RANGE_3); // Adjust sector and voltage range accordingly

    // Write the data to flash word by word
    uint32_t data_length = strlen(data) + 1;  // +1 to include the null terminator
    for (uint32_t i = 0; i < data_length; i += 4) {
        uint32_t word = 0xFFFFFFFF;

        // Copy up to 4 bytes into a word (to handle the last word which might be less than 4 bytes)
        memcpy(&word, &data[i], data_length - i >= 4 ? 4 : data_length - i);

        if (HAL_FLASH_Program(TYPEPROGRAM_WORD, address + i, word) != HAL_OK) {
            // Handle the error
            break;
        }
    }

    HAL_FLASH_Lock();
}

void Flash_Read_String(uint32_t address, char *buffer, uint32_t max_length) {
    uint32_t *flash_address = (uint32_t*)address;
    char *char_ptr = (char*)flash_address;

    // Read bytes until the null terminator is found or max_length is reached
    for (uint32_t i = 0; i < max_length; i++) {
        buffer[i] = char_ptr[i];
        if (buffer[i] == '\0') {
            break;
        }
    }
}


u8 extract_serial_number(const char *input, char *output) {
    const char *prefix = "U";  // The constant prefix to search for
    const char *start = strstr(input, prefix);

    if (start) {
        // Check if the found character is followed by digits
        bool is_valid = true;
        for (int i = 2; i < 5; i++) {  // Check the next 7 characters after 'A'
            if (start[i] < '0' || start[i] > '9') {
                is_valid = false;
                break;
            }
        }

        if (is_valid) {
            strncpy(output, start, 8);  // Copy "A0010104" to output (8 characters)
            output[5] = '\0';  // Null-terminate the output string
        } else {
            strcpy(output, "");  // No valid serial number found
        }
    } else {
        strcpy(output, "");  // No "A" found in the input
    }
    return 0;
}
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
u8 serialCommand(char *str)
{
             if(strstr((char *)str,"save"))
             {
              sprintf(str,"Serial Number Saved to Memory: %s \r",
            		  _serialN);
             // Write the string to flash memory
              __disable_irq();
              Flash_Write_String(FLASH_USER_START_ADDR, _serialN);
              __enable_irq();
             }
             else if(strstr((char *)str,"set"))
             {
               extract_serial_number(str,_serialN);
               sprintf(str,"New Serial Number is: %s \r",
            		   _serialN);


             }
             else if(strstr((char *)str,"get"))
             {

            	 sprintf(str,"Serial Number is: %s \r",
            	             		   _serialN);
             }
              else  {

         }
             TransmitCMDResponse(str);
             return 0;
 }
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
u8 serialCommandHelp(void)
{
	static u8 state=0;
	u8 returnValue=1;
	switch(state)
	{
	case 0:
	TransmitCMDResponse("     serial set UA001       -> (To set new srial Number) \r");
	state=1;
	break;
	case 1:
	TransmitCMDResponse("     serial save            -> (To save new serail to memory) \r");
	state=2;
	break;
	case 2:
	TransmitCMDResponse("     serial get             -> (Returns Device Serial Number) \r");
	state=0;
	returnValue=0;
	break;
	}

	return returnValue;
}
