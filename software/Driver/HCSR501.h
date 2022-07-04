/*
 * HCSR501.h
 *
 *  Created on: 11.12.2015
 *      Author: mbuder
 */

#ifndef I2C_DRIVER_HCSR501_H_
#define I2C_DRIVER_HCSR501_H_


#define HCSR501_VALID_MOTION_DELAY	(10) // Seconds



void HCSR501Init(void);
void HCSR501OnMotion(void);
bool HCSR501IsActive(void);
void HCSR501Enable(void);
void HCSR501Disable(void);


extern bool g_bHCSR501Enable;
extern bool g_bActiveMotionEvent;
extern bool g_bExitMotionEvent;

#endif /* I2C_DRIVER_HCSR501_H_ */
