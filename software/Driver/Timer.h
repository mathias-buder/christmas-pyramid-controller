/*
 * TimerHandler.h
 *
 *  Created on: 10.06.2015
 *      Author: mbuder
 */

#ifndef TIMERHANDLER_H_
#define TIMERHANDLER_H_

#include <stdint.h>

void TimerInit();
void Timer0AStart();
void Timer1AStart();
void Timer0AStop();
void Timer1AStop();
void Timer0ALoad(uint32_t ui32Value);
void Timer1ALoad(uint32_t ui32Value);
void Timer0AIntConnecte(void (*pfnHandler)(void));
void Timer1AIntConnecte(void (*pfnHandler)(void));
void Timer0AIntClear(void);
void Timer1AIntClear(void);
#endif /* TIMERHANDLER_H_ */
