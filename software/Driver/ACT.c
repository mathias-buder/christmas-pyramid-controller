/*
 * 8CHRLY.c
 *
 *  Created on: 29.11.2015
 *      Author: Local
 */


#include "ACT.h"

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_ssi.h"
#include "driverlib/ssi.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/udma.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"

//*****************************************************************************
//
//! \addtogroup dac_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// The I2C interface used by this driver.
//
//*****************************************************************************
#define ACT_GPIO_PERIPH                  (SYSCTL_PERIPH_GPIOB)
#define ACT_GPIO_BASE                    (GPIO_PORTB_BASE)
#define ACT_CH1                          (GPIO_PIN_0)
#define ACT_CH2                          (GPIO_PIN_1)
#define ACT_CH3                          (GPIO_PIN_2)
#define ACT_CH4                          (GPIO_PIN_3)
#define ACT_CH5                          (GPIO_PIN_4)
#define ACT_CH6                          (GPIO_PIN_5)
#define ACT_CH7                          (GPIO_PIN_6)
#define ACT_CH8                          (GPIO_PIN_7)
#define ACT_PINS                         (ACT_CH1 | ACT_CH2 | ACT_CH3 | \
                                          ACT_CH4 | ACT_CH5 | ACT_CH6 | \
                                          ACT_CH7 | ACT_CH8)

//*****************************************************************************
//
// Initialize DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
bool
ActEnbale(uint8_t ui8Channel)
{
    if(ui8Channel < 1 || ui8Channel > ACT_N_CH)
    {
        return false;
    }

    switch (ui8Channel)
    {
        case 1: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH1, 0); break;
        case 2: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH2, 0); break;
        case 3: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH3, 0); break;
        case 4: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH4, 0); break;
        case 5: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH5, 0); break;
        case 6: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH6, 0); break;
        case 7: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH7, 0); break;
        case 8: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH8, 0); break;

        default: GPIOPinWrite(ACT_GPIO_BASE, ACT_PINS, ACT_PINS); break;
    }

    return true;
}


//*****************************************************************************
//
// Initialize DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
void
ActEnbaleAll(void)
{
	GPIOPinWrite(ACT_GPIO_BASE, ACT_PINS, 0);
}

//*****************************************************************************
//
// Initialize DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
bool
ActDisable(uint8_t ui8Channel)
{
    if(ui8Channel < 1 || ui8Channel > ACT_N_CH)
    {
        return false;
    }

    switch (ui8Channel)
    {
        case 1: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH1, ACT_CH1); break;
        case 2: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH2, ACT_CH2); break;
        case 3: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH3, ACT_CH3); break;
        case 4: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH4, ACT_CH4); break;
        case 5: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH5, ACT_CH5); break;
        case 6: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH6, ACT_CH6); break;
        case 7: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH7, ACT_CH7); break;
        case 8: GPIOPinWrite(ACT_GPIO_BASE, ACT_CH8, ACT_CH8); break;

        default: GPIOPinWrite(ACT_GPIO_BASE, ACT_PINS, ACT_PINS); break;
    }

    return true;
}

//*****************************************************************************
//
// Initialize DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
void
ActDisableAll(void)
{
	GPIOPinWrite(ACT_GPIO_BASE, ACT_PINS, ACT_PINS);
}

//*****************************************************************************
//
// Initialize DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
void
ActInit(void)
{
    //
    // Enable the GPIO port
    //
    SysCtlPeripheralEnable(ACT_GPIO_PERIPH);

    //
    // Configure used pins as GPIO output
    //
    GPIOPinTypeGPIOOutput(ACT_GPIO_BASE, ACT_PINS);


    //
    // Disable all channels (active low)
    //
    GPIOPinWrite(ACT_GPIO_BASE, ACT_PINS, ACT_PINS);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
