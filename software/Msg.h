/*
 * msg.h
 *
 *  Created on: 16.10.2016
 *      Author: Local
 */

#ifndef MSG_H_
#define MSG_H_

#include "utils/uartstdio.h"

//*****************************************************************************
//
// This define prints an error message to the console
//
//*****************************************************************************

#define OkPrint					UARTprintf(" OK\n")



/*
#define ERRMSG(fms, ...)	do{	UARTprintf("\033[31;1m"); \
								UARTprintf("[ %s:%s():%i ] " fms, __FILE__,  __func__, __LINE__, __VA_ARGS__); \
								UARTprintf("\033[0m");} while(0)
*/

#define ERRMSG(fms, ...)	UARTprintf("[ ERROR %s:%s():%i ] " fms, __FILE__,  __func__, __LINE__, __VA_ARGS__)



//*****************************************************************************
//
// This define prints an standart message to the console
//
//*****************************************************************************
/*
#define SUCMSG(fms, ...)	do{	UARTprintf("\033[32;1m"); \
								UARTprintf("[ %s:%s():%i ] " fms, __FILE__,  __func__, __LINE__, __VA_ARGS__); \
								UARTprintf("\033[0m");} while(0)
*/

#define SUCMSG(fms, ...)	UARTprintf("[ %s:%s():%i ] " fms, __FILE__,  __func__, __LINE__, __VA_ARGS__)

//*****************************************************************************
//
// This define prints an standart message to the console
//
//*****************************************************************************
#define MSG(fms, ...)		UARTprintf("[ %s:%s():%i ] " fms, __FILE__,  __func__, __LINE__, __VA_ARGS__)

//*****************************************************************************
//
// This define prints an log message to the log-file
//
//*****************************************************************************
#define LOG(fms, ...)		do{	UARTprintf("\033[31;1m"); \
								UARTprintf("[ERROR] " fms "\n", __VA_ARGS__); \
								UARTprintf("\033[0m");} while(0)

#endif /* MSG_H_ */
