/*
 * TimerHandler.c
 *
 *  Created on: 10.06.2015
 *      Author: mbuder
 */
#include "Timer.h"
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "VS10xx.h"



//*****************************************************************************
//
//! \addtogroup TimerManager_api
//! @{
//
//*****************************************************************************

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
Timer0AStart(void)
{
	TimerEnable(TIMER0_BASE, TIMER_A);
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
Timer1AStart(void)
{
	TimerEnable(TIMER1_BASE, TIMER_A);
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
Timer0AStop(void)
{
	TimerDisable(TIMER0_BASE, TIMER_A);
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
Timer1AStop(void)
{
	TimerDisable(TIMER1_BASE, TIMER_A);
}

//*****************************************************************************
//
//! Load the timer
//!
//! \param ui8Duration in milliseconds (ms)
//!
//!  This function will write the register passed in ucAddr with the value
//!  passed in to ulData.  The data in ulData is actually 9 bits and the
//!  value in ucAddr is interpreted as 7 bits.
//!
//! \return none
//
//*****************************************************************************
void
Timer0ALoad(uint32_t ui32Value)
{
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Value);
}


//*****************************************************************************
//
//! Load the timer
//!
//! \param ui8Duration in milliseconds (ms)
//!
//!  This function will write the register passed in ucAddr with the value
//!  passed in to ulData.  The data in ulData is actually 9 bits and the
//!  value in ucAddr is interpreted as 7 bits.
//!
//! \return none
//
//*****************************************************************************
void
Timer1ALoad(uint32_t ui32Value)
{
	TimerLoadSet(TIMER1_BASE, TIMER_A, ui32Value);
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
Timer0AIntConnecte(void (*pfnHandler)(void))
{
	TimerIntRegister(TIMER0_BASE, TIMER_A, pfnHandler);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
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
Timer1AIntConnecte(void (*pfnHandler)(void))
{
	TimerIntRegister(TIMER1_BASE, TIMER_A, pfnHandler);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
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
Timer0AIntClear(void)
{
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
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
Timer1AIntClear(void)
{
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
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
void TimerInit()
{
	    //
	    // Enable the peripherals used by this example.
	    //
	    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

	    //
	    // Configure the two 32-bit periodic timers.
	    //
	    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);

	    //
	    // Load timer with initial counter value
	    //
	    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet());
	    TimerLoadSet(TIMER1_BASE, TIMER_A, SysCtlClockGet());
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
