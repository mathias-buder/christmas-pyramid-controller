/*
 * LEDs.c
 *
 *  Created on: 08.12.2015
 *      Author: M. Buder
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "LED.h"

//*****************************************************************************
//
//! \addtogroup dac_api
//! @{
//
//*****************************************************************************

// GPIO for LEDs
#define LED_ACK_ERR_GPIO_SYSCTL_PERIPH  	SYSCTL_PERIPH_GPIOF
#define LED_SD_GPIO_SYSCTL_PERIPH       	SYSCTL_PERIPH_GPIOD

#define LED_ACK_ERR_GPIO_PORT_BASE			GPIO_PORTF_BASE
#define LED_SD_GPIO_PORT_BASE				GPIO_PORTD_BASE

#define BIT_LED_ACK                       	GPIO_PIN_4
#define BIT_LED_ERR                      	GPIO_PIN_3
#define BIT_LED_SD_DETECT                   GPIO_PIN_6

uint8_t ui8LedAckToggleVal = 0;
uint8_t ui8LedErrToggleVal = 0;
uint8_t ui8LedCDToggleVal = 0;

//*****************************************************************************
//
// Get time and date from DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//****************************************************************************
void
LEDEnable(uint8_t ui8Led)
{
	switch (ui8Led) {
		case LED_ACK: GPIOPinWrite(LED_ACK_ERR_GPIO_PORT_BASE, BIT_LED_ACK, BIT_LED_ACK); break;
		case LED_ERR: GPIOPinWrite(LED_ACK_ERR_GPIO_PORT_BASE, BIT_LED_ERR, BIT_LED_ERR); break;
		case LED_SD_DETECT: GPIOPinWrite(LED_SD_GPIO_PORT_BASE, BIT_LED_SD_DETECT, BIT_LED_SD_DETECT); break;
	}

}

//*****************************************************************************
//
// Get time and date from DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//****************************************************************************
void
LEDDisable(uint8_t ui8Led)
{
		switch (ui8Led) {
			case LED_ACK: GPIOPinWrite(LED_ACK_ERR_GPIO_PORT_BASE, BIT_LED_ACK, 0); break;
			case LED_ERR: GPIOPinWrite(LED_ACK_ERR_GPIO_PORT_BASE, BIT_LED_ERR, 0); break;
			case LED_SD_DETECT: GPIOPinWrite(LED_SD_GPIO_PORT_BASE, BIT_LED_SD_DETECT, 0); break;
		}
}



//*****************************************************************************
//
// Get time and date from DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//****************************************************************************
void
LEDToggle(uint8_t ui8Led)
{
		switch (ui8Led) {
			case LED_ACK: ui8LedAckToggleVal ^= BIT_LED_ACK; GPIOPinWrite(LED_ACK_ERR_GPIO_PORT_BASE, BIT_LED_ACK, ui8LedAckToggleVal); break;
			case LED_ERR: ui8LedErrToggleVal ^= BIT_LED_ERR; GPIOPinWrite(LED_ACK_ERR_GPIO_PORT_BASE, BIT_LED_ERR, ui8LedErrToggleVal); break;
			case LED_SD_DETECT: ui8LedCDToggleVal ^= BIT_LED_SD_DETECT; GPIOPinWrite(LED_SD_GPIO_PORT_BASE, BIT_LED_SD_DETECT, ui8LedCDToggleVal); break;
		}
}


//*****************************************************************************
//
// Get time and date from DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//****************************************************************************
void
LEDInit(void)
{
    //
    // Enable the peripherals used by this driver
    //
	SysCtlPeripheralEnable(LED_ACK_ERR_GPIO_SYSCTL_PERIPH);
	SysCtlPeripheralEnable(LED_SD_GPIO_SYSCTL_PERIPH);

    //
    // Configure VS1003 xRST ,xDCS and xCS control pins as GPIO output
    //
    GPIOPinTypeGPIOOutput(LED_ACK_ERR_GPIO_PORT_BASE, BIT_LED_ACK | BIT_LED_ERR);
    GPIOPinTypeGPIOOutput(LED_SD_GPIO_PORT_BASE, BIT_LED_SD_DETECT);


    //
    // Disable all LEDs
    //
    GPIOPinWrite(LED_ACK_ERR_GPIO_PORT_BASE, BIT_LED_ACK | BIT_LED_ERR, 0);
    GPIOPinWrite(LED_SD_GPIO_PORT_BASE, BIT_LED_SD_DETECT, 0);

}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
