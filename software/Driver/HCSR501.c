/*
 * HCSR501.c
 *
 *  Created on: 11.12.2015
 *      Author: mbuder
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
#include "HCSR501.h"
#include "../Manager/AudioManager.h"
#include "VS10xx.h"
#include "Timer.h"
#include "../Msg.h"
#include "../System.h"

//*****************************************************************************
//
//! \addtogroup TimerManager_api
//! @{
//
//*****************************************************************************
//*****************************************************************************
//
// Printf functions used in this file
//
//*****************************************************************************
#define FILE_ID				("HCSR")
#define eprintf(fms, ...)	do{	UARTprintf("\033[31;1m"); \
								UARTprintf("[ %s ] ERROR " fms, FILE_ID, __VA_ARGS__); \
								UARTprintf("\033[0m");} while(0)

#define dprintf(fms, ...)	UARTprintf("[ %s ] " fms, FILE_ID, __VA_ARGS__); \

//*****************************************************************************
//
//! Peripheral definitions used in this driver
//
//*****************************************************************************
// GPIO for UART pins
#define HCSR501_GPIO_SYSCTL_PERIPH		SYSCTL_PERIPH_GPIOD
#define HCSR501_GPIO_PORT_BASE			GPIO_PORTD_BASE
#define HCSR501_PIN						GPIO_PIN_1
#define HCSR501_INT_PIN					GPIO_INT_PIN_1

//*****************************************************************************
//
//! Gloabal definitions used in this driver
//
//*****************************************************************************
bool g_bHCSR501Enable;
bool g_bActiveMotionEvent;
bool g_bExitMotionEvent;
uint8_t g_ui8Seconds;

//*****************************************************************************
//
//! Writes a register in the DS3231 RTC.
//!
//! \param ucRegister is the offset to the register to write.
//! \param ulData is the data to be written to the DAC register.
//!
//!  This function will write the register passed in ucAddr with the value
//!  passed in to ulData.  The data in ulData is actually 9 bits and the
//!  value in ucAddr is interpreted as 7 bits.
//!
//! \return True on success or false on error.
//
//*****************************************************************************
void
HCSR501TimerIntHandler(void)
{
	Timer1AIntClear();

	dprintf("Bewegungsereignis fuer %i/%i (s)\n", g_ui8Seconds, g_sSystem.ui8DefaultMotionDelay);

	if(GPIOPinRead(HCSR501_GPIO_PORT_BASE, HCSR501_PIN))
	{
		g_ui8Seconds++;

		if(g_ui8Seconds > g_sSystem.ui8DefaultMotionDelay)
		{
			g_bExitMotionEvent = false;
			g_bActiveMotionEvent = true;

			Timer1AStop();

			dprintf("Gueltiges Bewegungsereigniss.\n", 0);
		}
	}
}



//*****************************************************************************
//
//! Writes a register in the DS3231 RTC.
//!
//! \param ucRegister is the offset to the register to write.
//! \param ulData is the data to be written to the DAC register.
//!
//!  This function will write the register passed in ucAddr with the value
//!  passed in to ulData.  The data in ulData is actually 9 bits and the
//!  value in ucAddr is interpreted as 7 bits.
//!
//! \return True on success or false on error.
//
//*****************************************************************************
void
HCSR501OnMotion(void)
{
	if(GPIOPinRead(HCSR501_GPIO_PORT_BASE, HCSR501_PIN))
	{
		//
		// Motion event
		//
		dprintf("Bewegungsereigniss erkannt.\n", 0);
		g_ui8Seconds = 1;
		Timer1AStart();
	}
	else
	{
		//
		// Exit motion event
		//
		if(g_bActiveMotionEvent)
		{
			g_bExitMotionEvent = true;
		}

		Timer1AStop();
		g_bActiveMotionEvent = false;
		dprintf("Bewegungsereigniss beendet.\n", 0);
	}
}

//*****************************************************************************
//
//! Writes a register in the DS3231 RTC.
//!
//! \param ucRegister is the offset to the register to write.
//! \param ulData is the data to be written to the DAC register.
//!
//!  This function will write the register passed in ucAddr with the value
//!  passed in to ulData.  The data in ulData is actually 9 bits and the
//!  value in ucAddr is interpreted as 7 bits.
//!
//! \return True on success or false on error.
//
//*****************************************************************************
bool
HCSR501IsActive(void)
{
	return (GPIOPinRead(HCSR501_GPIO_PORT_BASE, HCSR501_PIN) ? 1 : 0);
}


//*****************************************************************************
//
//! Writes a register in the DS3231 RTC.
//!
//! \param ucRegister is the offset to the register to write.
//! \param ulData is the data to be written to the DAC register.
//!
//!  This function will write the register passed in ucAddr with the value
//!  passed in to ulData.  The data in ulData is actually 9 bits and the
//!  value in ucAddr is interpreted as 7 bits.
//!
//! \return True on success or false on error.
//
//*****************************************************************************
void
HCSR501Enable(void)
{
    //
    // Enable interrupts for the specified pin
    //
    GPIOIntEnable(HCSR501_GPIO_PORT_BASE, HCSR501_INT_PIN);

    GPIOIntClear(HCSR501_GPIO_PORT_BASE, HCSR501_INT_PIN);

    g_bHCSR501Enable = true;
    dprintf("Bewegungssensor Aktiviert.\n", 0);
}



//*****************************************************************************
//
//! Writes a register in the DS3231 RTC.
//!
//! \param ucRegister is the offset to the register to write.
//! \param ulData is the data to be written to the DAC register.
//!
//!  This function will write the register passed in ucAddr with the value
//!  passed in to ulData.  The data in ulData is actually 9 bits and the
//!  value in ucAddr is interpreted as 7 bits.
//!
//! \return True on success or false on error.
//
//*****************************************************************************
void
HCSR501Disable(void)
{
	GPIOIntClear(HCSR501_GPIO_PORT_BASE, HCSR501_INT_PIN);

    //
    // Enable interrupts for the specified pin
    //
    GPIOIntDisable(HCSR501_GPIO_PORT_BASE, HCSR501_INT_PIN);

    g_bHCSR501Enable = false;
    dprintf("Bewegungssensor deaktiviert.\n", 0);

}


//*****************************************************************************
//
//! Writes a register in the DS3231 RTC.
//!
//! \param ucRegister is the offset to the register to write.
//! \param ulData is the data to be written to the DAC register.
//!
//!  This function will write the register passed in ucAddr with the value
//!  passed in to ulData.  The data in ulData is actually 9 bits and the
//!  value in ucAddr is interpreted as 7 bits.
//!
//! \return True on success or false on error.
//
//*****************************************************************************
void
HCSR501Init(void)
{
	dprintf("Initialisiere HCSR501 Bewegungssensor ...", 0);

	g_bHCSR501Enable = false;
	g_bActiveMotionEvent = false;
	g_bExitMotionEvent = false;
	g_ui8Seconds = 0;

	//
	// Enable peripherals used by this driver
	//
    SysCtlPeripheralEnable(HCSR501_GPIO_SYSCTL_PERIPH);

    //
	// Set coresponding pins to be a GPIO input
	//
    GPIOPinTypeGPIOInput(HCSR501_GPIO_PORT_BASE, HCSR501_PIN);

    //
    // Configure interrupt pin and trigger mode
    //
    GPIOIntTypeSet(HCSR501_GPIO_PORT_BASE, HCSR501_INT_PIN, GPIO_BOTH_EDGES);

    //
    // Disable interrupts for the specified pin
    //
    GPIOIntDisable(HCSR501_GPIO_PORT_BASE, HCSR501_INT_PIN);

    //
    // Connect the timer interrupt handler
    //
    Timer1AIntConnecte(HCSR501TimerIntHandler);

    OkPrint;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************



