/*
 * RTE_MB.h
 *
 *  Created on: Jan 30, 2025
 *      Author: 98912
 */

#ifndef RTE_RTE_MB_RTE_MB_H_
#define RTE_RTE_MB_RTE_MB_H_
#include <stdint.h>
extern uint32_t mbTick;


typedef struct {
    int id;
    int (*senReq)(void);                     // Function pointer: int function(void)
    void (*resProcess)(char *res, int Len);  // Function pointer: void function(char*, int)
} MB_Slave_Struct;


/*!
 **************************************************************************************************
 *
 *  @fn         int RTE_MB_Mng(void)
 *
 *  @par        Public function to Manage MB
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
int RTE_MB_Mng(void) ;
/*!
 **************************************************************************************************
 *
 *  @fn         void MB_Manager_RegisterSlave(MB_Slave_Struct slave)
 *
 *  @par        Public function to register a slave
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
void MB_Manager_RegisterSlave(MB_Slave_Struct slave) ;
/*!
 **************************************************************************************************
 *
 *  @fn         int RTE_MB_Rec_Mng(uint8_t *res, int Len)
 *
 *  @par        Public function to Analyze MB
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
int RTE_MB_Rec_Mng(uint8_t *res,int Len) ;

#endif /* RTE_RTE_MB_RTE_MB_H_ */
