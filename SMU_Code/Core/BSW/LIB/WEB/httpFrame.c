/*
* hhtpFrame.c
*
*  Created on: Jan 8, 2025
*      Author: Kh.166
*/
#include "httpFrame.h"
#include "dbg.h"
#include "mntdata.h"
#include <stdlib.h>
#include <time.h>
#include "../../SVC/COM/MDM/MdmSrv.h"
#include "iflash.h"

//static int dataPackJson(char *str);


/**********************************_startFrame*************************************/
void startJsonFrame( char *str, DateTime tm)
{
  
  sprintf((char *)str,"{\r\"serial_number\":\"%s\",\r"
          "\"date\":\"%04d-%02d-%02d\",\r"
            "\"time\":\"%02d:%02d:%02d\",\r\"data\":{\r",_serialN,tm.year,tm.month,tm.day,tm.hour,tm.minute,tm.second);
}
/**********************************_addData2Frame*************************************/
int   addDataToJsonFrame(char *str){
  // static variables to keep our place
  static size_t tableIndex   = 0;
  static size_t itemIndex    = 0;
  static size_t itemsRead    = 0;
  static size_t totalItems   = 0;
  static int    initialized  = 0;
  static int live=0;
  
  const Database_Type *mDb = getMntDatabase();
  size_t numTables = getSizeOfRgsModule();
  
  // If not initialized, calculate total number of items
  if (!initialized)
  {
    for (size_t t = 0; t < numTables; t++)
    {
      totalItems += mDb[t].mDataSize;
    }
    initialized = 1;
  }
  
  // If we've already read all items, return empty JSON
  if (itemsRead >= totalItems || totalItems == 0)
  {   itemsRead=0;
  itemIndex=0;
  tableIndex=0;
  return 2;
  }
  
  
  // Decide how many items to fetch: up to 6, or the remainder
  size_t itemsRemaining = totalItems - itemsRead;
  size_t itemsToFetch   = (itemsRemaining < 6) ? itemsRemaining : 6;
  
  for (size_t i = 0; i < itemsToFetch; i++)
  {
    // If we are at the end of current table, move to next
    while (tableIndex < numTables &&
           itemIndex >= mDb[tableIndex].mDataSize)
    {
      tableIndex++;
      itemIndex = 0;
    }
    
    // If we are past the last table, it means no more items
    if (tableIndex >= numTables)
    {
      // Shouldn't happen unless totalItems changed on the fly,
      // but let's handle it safely:
      break;
    }
    
    MntDataType *item = &mDb[tableIndex].mData[itemIndex];
    
    // Format the JSON field
    // "V1": "Name*value*Unit",
    char entry[500];
    if(0==strcmp(item->name,"Live"))
       item->value=live++;
    snprintf(entry, sizeof(entry),
             "    \"V%d\": \"%s*%.1f*%s\"",
             i + 1,
             item->name,
             item->value,
             item->unit);
    
    // Comma except for the last item
    if (i < itemsToFetch - 1)
      strncat(entry, ",\r", sizeof(entry) - strlen(entry) - 1);
    else
      strncat(entry, "\r", sizeof(entry) - strlen(entry) - 1);
    
    // Append to main buffer
    strncat(str, entry, BUFFER_SIZE - strlen(str) - 1);
    
    // Move on
    itemIndex++;
    itemsRead++;
  }
  
  // Close JSON
  strncat(str, "}\r}", BUFFER_SIZE - strlen(str) - 1);
  //TransmitDebug(str);
  return 1;
  
}





