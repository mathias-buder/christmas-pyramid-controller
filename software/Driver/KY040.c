/*
 * KY040.c
 *
 *  Created on: 22.02.2016
 *      Author: Mathias
 */


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "utils/uartstdio.h"
#include "KY040.h"



/***************************************************
 * Peripheral definitions for VS1003 SPI interface
 * *************************************************/
// GPIO for UART pins
#define KY040_GPIO_SYSCTL_PERIPH		SYSCTL_PERIPH_GPIOE
#define KY040_GPIO_PORT_BASE			GPIO_PORTE_BASE
#define KY040_SW						GPIO_PIN_1
#define KY040_INT_SW					GPIO_INT_PIN_1
#define KY040_DR						GPIO_PIN_2
#define KY040_CLK						GPIO_PIN_3
#define KY040_INT_CLK					GPIO_INT_PIN_3



uint16_t g_ui16CyclesLow = 2000;
uint32_t g_ui32CyclesDelay = 1000000;

void
KY040IntHandlerSet(void (*pfnIntHandler)(void))
{
    //
    //	Set interrupt handler function pointer
    //
    GPIOIntRegister(KY040_GPIO_PORT_BASE, pfnIntHandler);

}


void KY040IntHandler(void)
{
	uint16_t ui8Cnt = 0;
	//GPIOIntClear(KY040_GPIO_PORT_BASE, KY040_INT_CLK | KY040_INT_SW);
	GPIOIntDisable(KY040_GPIO_PORT_BASE, KY040_INT_CLK | KY040_INT_SW);

	// UARTprintf("KY040IntHandler\n");


	while(!GPIOPinRead(KY040_GPIO_PORT_BASE, KY040_CLK))
	{
		if(ui8Cnt++ > g_ui16CyclesLow)
		{
			if(GPIOPinRead(KY040_GPIO_PORT_BASE, KY040_DR))
			{
				UARTprintf("ENCODER CCW\n");
				break;
			}
			else
			{
				UARTprintf("ENCODER CW\n");
				break;
			}
		}
	}

	SysCtlDelay(g_ui32CyclesDelay);
	GPIOIntClear(KY040_GPIO_PORT_BASE, KY040_INT_CLK | KY040_INT_SW);
	GPIOIntEnable(KY040_GPIO_PORT_BASE, KY040_INT_CLK | KY040_INT_SW);
}


/*
void
KY040IntFlagClear(void)
{
	GPIOIntClear(KY040_GPIO_PORT_BASE, KY040_INT_PINS);
}


void KY040IntEnable(void)
{
	//
	// Enable interrupt
	//
	GPIOIntEnable(KY040_GPIO_PORT_BASE, KY040_INT_PINS);
}

void KY040IntDisable(void)
{
	//
	// Disable interrupt
	//
	GPIOIntDisable(KY040_GPIO_PORT_BASE, KY040_INT_PINS);
}
*/

void
KY040Init(void)
{
    SysCtlPeripheralEnable(KY040_GPIO_SYSCTL_PERIPH);

    //
	// Set coresponding pins to be a GPIO input
	//
    GPIOPinTypeGPIOInput(KY040_GPIO_PORT_BASE, KY040_DR);
    GPIOPinTypeGPIOInput(KY040_GPIO_PORT_BASE, KY040_CLK);
    GPIOPinTypeGPIOInput(KY040_GPIO_PORT_BASE, KY040_SW);

    //
    // Configure interrupt pin and trigger mode
    //
    GPIOIntTypeSet(KY040_GPIO_PORT_BASE, KY040_INT_CLK | KY040_INT_SW, GPIO_FALLING_EDGE);

    // Hier im IsrWrapper registrieren
    GPIOIntRegister(KY040_GPIO_PORT_BASE, KY040IntHandler);

    GPIOIntEnable(KY040_GPIO_PORT_BASE, KY040_INT_CLK | KY040_INT_SW);
}
