/*
 * LEDs.h
 *
 *  Created on: 08.12.2015
 *      Author: Local
 */

#ifndef I2C_DRIVER_LED_H_
#define I2C_DRIVER_LED_H_


#define LED_ERR			0
#define LED_ACK			1
#define LED_SD_DETECT	2



void LEDInit(void);
void LEDEnable(uint8_t ui8Led);
void LEDDisable(uint8_t ui8Led);
void LEDToggle(uint8_t ui8Led);


#endif /* I2C_DRIVER_LED_H_ */
