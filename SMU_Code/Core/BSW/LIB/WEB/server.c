/*
 * server.c
 *
 *  Created on: Oct 15, 2025
 *      Author: KHAJEHHOD-GD09
 */

#include "server.h"
#include <string.h>
#include "mntdata.h"

u8 gUrl[URL_SIZE];

void server_select(void)
{
	RefDataType *rData=getRefData();

	if(rData[INV_SERVER_INDEX].value == 0)
	{
       strncpy(gUrl,URL_CA,URL_SIZE);
		//strncpy(gUrl,URL_IR,URL_SIZE);
	}
	else
	{
		strncpy(gUrl,URL_IR,URL_SIZE);
	}
}


void server_return_url(u8 *url)
{
	strncpy(url,gUrl,URL_SIZE);
}
