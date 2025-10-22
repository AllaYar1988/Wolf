/*
* sim7600http.h
*
*  Created on: Jan 7, 2025
*      Author: 98912
*/

#ifndef _MDMSRV_H_
#define _MDMSRV_H_
#include "main.h"      // For sprintf, printf, etc.

#include <stdio.h>      // For sprintf, printf, etc.
#include <string.h>     // For strlen, memset
#include <stdlib.h>     // For atoi
#include <stdint.h>     // For uint8_t, etc.
#include <stdbool.h>
#include "iflash.h"
#include "Platform.h"


#define BUFFER_SIZE 500
#define CMD_BUFFER_SIZE 256

#define MIN_DATA_LEN 20
#define MAX_DATA_LEN 500

#define REST_WAIT_TIME 30
#define SOFT_RESET_TIMEOUT 180
#define HARD_RESET_TIMEOUT 400




#define MDM_RESET_CMD "AT+CFUN=1,1\r"
#define MDM_SKIP_CMD  "+++"

#define APN "hologram"


// ---------------------------------------------------------------------------
// 1) Define an enum to replace "case" numbers
// ---------------------------------------------------------------------------
typedef enum
{
  CMD_ATE              = 1,    // old case 1
  CMD_AT_CTZU          = 100,  // old case 100
  CMD_AT_CCLOCK_REQ    = 101,  // old case 101
  CMD_CHECK_TIME_TAG   = 102,  // old case 102
  CMD_CGATT            = 2,    // old case 2
  CMD_HTTPINIT         = 3,    // old case 3
  CMD_HTTPPARA_URL     = 6,    // old case 6
  CMD_HTTPPARA_CONTENT = 7,    // old case 7
  CMD_HTTPPARA_CID     = 8,    // old case 8
  CMD_SEND_DATA_LEN    = 9,    // old case 9
  CMD_SEND_DATA        = 10,   // old case 10
  CMD_HTTP_ACTION      = 11,   // old case 11
  CMD_WAIT_FOR_DOWNLOAD= 12,
  CMD_WAIT_FOR_SEND_OK = 13,
  CMD_RESET_MDM        = 14,
  CMD_RESET_MDM_WAIT   = 15,
  CMD_HTTP_INIT_WAIT   = 16,
  CMD_CGDCONT          = 17,
  CMD_CGACT            = 18,
  CMD_WAIT_RESPONSE    = 109,  // old case 109
  CMD_READ_DATA        = 110,  // old case 110
  CMD_WAIT_READ        = 111,  // old case 111
  CMD_HTTP_TERM        = 112,   // old case 12
  CMD_FINISH           = 113    // old case 17
} eSendCommand;

// ---------------------------------------------------------------------------
// 2) Create an array (or struct array) of AT commands and lengths.
//    This example uses a struct array for clarity (cmd + length).
// ---------------------------------------------------------------------------
// A union that can store the command in two possible ways:
//   1) asPtr: pointer to a string literal
//   2) asArray: character array (sized as needed)

// 1) We define a small struct for each command
typedef struct {
  u8 *command;
} ATCommandStruct;

// 2) We define a union that can hold (for example) 12 commands
typedef union {
  ATCommandStruct list[12];
} ATCommandUnion;

extern  int _readreq;
extern char _serialN[10];
extern int _waittosend;
extern UART_HandleTypeDef huart1;
typedef struct {
  uint32_t simSoft;
  uint32_t simHard;
  uint32_t simSkip;
  uint32_t resetHold;
  uint32_t flag;
  
}TIME_OUT;

typedef struct {
  uint8_t busy;
  uint8_t response;
  //char rData[BUFFER_SIZE];     // 500 bytes
  char api[50];
  char sData[BUFFER_SIZE];     // 500 bytes
} GPRS_HandleTypeDef;

typedef struct page {
  unsigned int current;
  unsigned int total;
  
} page_Handletypedef;

extern GPRS_HandleTypeDef mdmGprs;

extern RTC_HandleTypeDef hrtc;


void ModuleHandle(GPRS_HandleTypeDef *mGprs);
/*!
 **************************************************************************************************
 *
 *  @fn         int MdmSrv_CheckRefUpdate(uint8_t *str,int *readreq);
 *
 *  @par        This function to check Update Ref Data.
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
int MdmSrv_CheckRefUpdate(uint8_t *str,int *readreq);

int process_AT_command(void);
void mdmResProcess(void);
/*!
 **************************************************************************************************
 *
 *  @fn         int MdmSrv_UpdateReferences(char *str, page_Handletypedef *page, int *readreq)
 *
 *  @par        This function to Update Ref Data.
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
int MdmSrv_UpdateReferences(uint8_t *str, page_Handletypedef *page, int *readreq) ;
/*!
 **************************************************************************************************
 *
 *  @fn         u8 MdmSrv_GetWebRefIdSts(void)
 *
 *  @par        This function to get Web RefId status
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
u8 MdmSrv_GetWebRefIdSts(void);
/*!
 **************************************************************************************************
 *
 *  @fn         void MdmSrv_SetWebRefIdSts(u8 u8Value)
 *
 *  @par        This function to set Web RefId status
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
void MdmSrv_SetWebRefIdSts(u8 u8Value);
/*!
 **************************************************************************************************
 *
 *  @fn         u16 MdmSrv_GetWebRefId(void)
 *
 *  @par        This function to get Web RefId
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
u16 MdmSrv_GetWebRefId(void);


u8 simCommand(char *str);
u8 simCommandHelp(void);
u8 atDirectCommand(char *str);
u8 atDirectCommandHelp(void);


#endif /* SRC_SIM7600HTTP_H_ */
