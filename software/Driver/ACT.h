/*
 * 8CHRLY.h
 *
 *  Created on: 29.11.2015
 *      Author: Local
 */

#ifndef I2C_DRIVER_ACT_H_
#define I2C_DRIVER_ACT_H_

#include <stdint.h>
#include <stdbool.h>

#define ACT_N_CH		(8)

//*****************************************************************************
//
// Prototypes APIs.
//
//*****************************************************************************
void ActInit(void);
bool ActEnbale(uint8_t ui8Channel);
void ActEnbaleAll(void);
bool ActDisable(uint8_t ui8Channel);
void ActDisableAll(void);


#endif /* I2C_DRIVER_ACT_H_ */
