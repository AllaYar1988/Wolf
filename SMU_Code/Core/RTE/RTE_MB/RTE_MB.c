/*
* RTE_MB.c
*
*  Created on: Jan 30, 2025
*      Author: A. Moazami
*/

#include "RTE_MB.h"
#include "S2_CH.h"
#include "S5_VGBR.h"
#include "S1_INV.h"
#include "S3_HMI.h"
#include <stdlib.h>
#include "modbus.h"
#include "S6_WEB.h"
#include "mntdata.h"
#include <stdint.h>

uint32_t mbTick=0;
uint32_t mbTimeout=0;
uint32_t mbSlaveTimeout=0;
// Define the linked list node (internally, not in the header)
typedef struct MB_Node {
  MB_Slave_Struct entry;
  struct MB_Node* next;
} MB_Node;

// Keep this static so no external file can access it directly
static MB_Node *list_head = NULL;

static MB_Node* create_node(MB_Slave_Struct data) ;


static void MB_UpdateStatus(void);

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
int RTE_MB_Mng(void)
{
  int status=1;
  static int state=0;
  static MB_Node* current = NULL;  // Static variables retain their value between function calls.
  
  switch (state)
  {
  case 0:        
    S1_INV_registerToMbNode();
    S2_CH_registerToMbNode();
    S3_HMI_registerToMbNode();
    //S5_VGBR_registerToMbNode();
    S6_WEB_registerToMbNode();
    current = list_head;
    mbSlaveTimeout=HAL_GetTick();
    mbTimeout=HAL_GetTick();
    //RTE_MB_Rec_Mng("Init",0);
    state=1;
    break;
  case 1:
    
    // Call the senReq function
    if(0== current->entry.senReq() ||(HAL_GetTick()- mbSlaveTimeout)>10000 ){
      // Move to the next node; wrap if at end
      current = (current->next) ? current->next : list_head;
      status=0;
      mbSlaveTimeout=HAL_GetTick();
    }
    if((HAL_GetTick()- mbTimeout)>30000 ){
    	mbTimeout=HAL_GetTick();
    	 state=2;
    }
    state=3;
    break;
  case 2:
	  _modbusMaster.Busy=0;
	  _modbusMaster.byteCount=0;
	  _modbusMaster.dataEnd=0;
	  _modbusMaster.firstByte=0;
	  memset(_modbusMaster.receiveDataArray,0,256);

	  state=1;
	  break;
  case 3:
	  MB_UpdateStatus();
	  state=1;

	  break;
  }
  
  return status;
}
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
int RTE_MB_Rec_Mng(uint8_t *res, int Len)
{
  MB_Node *current = list_head;  // start from the head each time
  while (current != NULL) {
    if (res[0] == current->entry.id) {
      current->entry.resProcess((char *)res, Len);
      mbTimeout=HAL_GetTick();
      return 0;  // success
    }
    current = current->next;
  }
  return 1; // not found
}

/*!
 **************************************************************************************************
 *
 *  @fn         static MB_Node* create_node(MB_Slave_Struct data)
 *
 *  @par         Helper to create a node
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
static MB_Node* create_node(MB_Slave_Struct data)
{
  MB_Node* new_node = (MB_Node*)malloc(sizeof(MB_Node));
  if (!new_node) {
    return NULL;
  }
  new_node->entry = data;
  new_node->next = NULL;
  return new_node;
}
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
void MB_Manager_RegisterSlave(MB_Slave_Struct slave)
{
  MB_Node* new_node = create_node(slave);
  if (!new_node) return;
  
  if (!list_head) {
    list_head = new_node;
  } else {
    MB_Node* current = list_head;
    while (current->next) {
      current = current->next;
    }
    current->next = new_node;
  }
}
/*!
 **************************************************************************************************
 *
 *  @fn         static void MB_UpdateStatus(void)
 *
 *  @par
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
static void MB_UpdateStatus(void)
{
   u16 u16LocalRemote=_memoryMap[39];
   u16 u16IsAnyCfgSet=(u16)getRefIDValue();

if(u16LocalRemote>u16IsAnyCfgSet)
{
	if(1==S1_INV_getUpdateStatus() && 1==S2_CH_getUpdateStatus())
	{
		S1_INV_setUpdateStatus(0);
		S2_CH_setUpdateStatus(0);
		setRefIDValue((u16)u16LocalRemote);

	}
}
}

