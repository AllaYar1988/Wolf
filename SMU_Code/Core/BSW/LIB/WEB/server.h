/*
 * server.h
 *
 *  Created on: Oct 15, 2025
 *      Author: Allahyar Moazami
 */

#ifndef BSW_LIB_WEB_SERVER_H_
#define BSW_LIB_WEB_SERVER_H_
#include "platform.h"

#define URL_CA "https://enerbundsmu.com"
#define URL_IR "https://uapelsmu.ir"

#define URL URL_IR
#define URL_SIZE 50

#define GET_REF_API "api/get-reference"
#define SEND_CH_API "api/send-chanel"


void server_select(void);


void server_return_url(u8 *url);


#endif /* BSW_LIB_WEB_SERVER_H_ */
