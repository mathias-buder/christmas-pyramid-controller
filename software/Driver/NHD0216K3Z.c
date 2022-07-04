/*
 * NHD0216K3Z.c
 *
 *  Created on: 11.12.2015
 *      Author: mbuder
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
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
#include "NHD0216K3Z.h"


/******************************************************
 * Peripheral definitions for NHD0216K3 UART interface
* ****************************************************/

// UART port
#define NHD0216K3_UART_SYSCTL_PERIPH            SYSCTL_PERIPH_UART2
#define NHD0216K3_UART_BASE                     UART2_BASE

// GPIO for UART pins
#define NHD0216K3_UART_GPIO_SYSCTL_PERIPH       SYSCTL_PERIPH_GPIOD
#define NHD0216K3_UART_GPIO_PORT_BASE           GPIO_PORTD_BASE
#define NHD0216K3_PINCFG_UARTTX					GPIO_PD7_U2TX
#define NHD0216K3_UART_TX						GPIO_PIN_7

// UART settings
#define NHD0216K3_BAUDRATE						9600


//*****************************************************************************
//
//! Plays back an audio file.
//!
//! \param readFp is the offset to the register to write.
//!
//!  This function records an audio file in Ogg, MP3, or WAV formats.e
//!  If recording in WAV format, it updates the RIFF length headers
//!  after recording has finished.
//!
//! \return None.
//
//*****************************************************************************
void
NHD0216K3ZSendData(uint8_t *pui8Buffer, uint32_t ui32Count)
{
    //
    // Loop while there are more characters to send.
    //
    while(ui32Count--)
    {
        //
        // Write the next character to the UART.
        //
        UARTCharPut(NHD0216K3_UART_BASE, *pui8Buffer++);
    }
}


//*****************************************************************************
//
//! Plays back an audio file.
//!
//! \param readFp is the offset to the register to write.
//!
//!  This function records an audio file in Ogg, MP3, or WAV formats.e
//!  If recording in WAV format, it updates the RIFF length headers
//!  after recording has finished.
//!
//! \return None.
//
//*****************************************************************************
void
NHD0216K3ZSendCommand(uint8_t ui8Command, int8_t i8Val)
{
	uint8_t pui8Data[NHD0216K3Z_COMMAND_SIZE];

	pui8Data[NHD0216K3Z_PREFIX] = NHD0216K3Z_COMMAND_PREFIX;
	pui8Data[NHD0216K3Z_COMMAND] = ui8Command;

	if(i8Val >= 0)
	{
		pui8Data[NHD0216K3Z_VAL] = i8Val;
		NHD0216K3ZSendData(pui8Data, NHD0216K3Z_COMMAND_SIZE);
	}
	else
	{
		NHD0216K3ZSendData(pui8Data, NHD0216K3Z_COMMAND_SIZE-1);
	}
}



//*****************************************************************************
//
//! Plays back an audio file.
//!
//! \param readFp is the offset to the register to write.
//!
//!  This function records an audio file in Ogg, MP3, or WAV formats.e
//!  If recording in WAV format, it updates the RIFF length headers
//!  after recording has finished.
//!
//! \return None.
//
//*****************************************************************************
void
NHD0216K3ZPrintText(uint8_t* pui8Data, uint8_t ui8Row, uint8_t ui8Aligment)
{
	uint8_t ui8StrLen;
	uint8_t ui8StartPos;

	//
	// Get sting length
	//
	ui8StrLen = strlen((const char*)  pui8Data);

	//
	// Determine curser postion according to aligment flag
	//
	switch (ui8Aligment) {
		case LEFT: ui8StartPos = 0; break;
		case RIGHT: ui8StartPos = NHD0216K3_COLUMNS - ui8StrLen; break;
		case CENTER: ui8StartPos = (NHD0216K3_COLUMNS - ui8StrLen)/2; break;
		default: ui8StartPos = 0; break;
	}

	//
	// Clear screen
	//
	//NHD0216K3ZSendCommand(NHD0216K3Z_CLEAR_SCREEN, -1);

	//
	// Set curser to specified row
	//
	switch (ui8Row)
	{
		case 1: NHD0216K3ZSendCommand(NHD0216K3Z_CURSOR_SET, ui8StartPos); break;
		case 2: NHD0216K3ZSendCommand(NHD0216K3Z_CURSOR_SET, NHD0216K3_ROW_2_ADDR + ui8StartPos); break;
		default: NHD0216K3ZSendCommand(NHD0216K3Z_CURSOR_SET, ui8StartPos); break;
	}

	//
	// Write data to display
	//
	NHD0216K3ZSendData(pui8Data, ui8StrLen);

}


//*****************************************************************************
//
//! Plays back an audio file.
//!
//! \param readFp is the offset to the register to write.
//!
//!  This function records an audio file in Ogg, MP3, or WAV formats.e
//!  If recording in WAV format, it updates the RIFF length headers
//!  after recording has finished.
//!
//! \return None.
//
//*****************************************************************************
void
NHD0216K3ZOff(void)
{
	NHD0216K3ZSendCommand(NHD0216K3Z_OFF, -1);
}


//*****************************************************************************
//
//! Plays back an audio file.
//!
//! \param readFp is the offset to the register to write.
//!
//!  This function records an audio file in Ogg, MP3, or WAV formats.e
//!  If recording in WAV format, it updates the RIFF length headers
//!  after recording has finished.
//!
//! \return None.
//
//*****************************************************************************
void
NHD0216K3ZOn(void)
{
	NHD0216K3ZSendCommand(NHD0216K3Z_ON, -1);
}




//*****************************************************************************
//
//! Plays back an audio file.
//!
//! \param readFp is the offset to the register to write.
//!
//!  This function records an audio file in Ogg, MP3, or WAV formats.e
//!  If recording in WAV format, it updates the RIFF length headers
//!  after recording has finished.
//!
//! \return None.
//
//*****************************************************************************
void
NHD0216K3ZSetBrightness(uint8_t ui8Brightness)
{
	if(ui8Brightness >= 1 && ui8Brightness <= 8)
	{
		NHD0216K3ZSendCommand(NHD0216K3Z_BRIGHTNESS_SET, ui8Brightness);
	}
}



//*****************************************************************************
//
//! Plays back an audio file.
//!
//! \param readFp is the offset to the register to write.
//!
//!  This function records an audio file in Ogg, MP3, or WAV formats.e
//!  If recording in WAV format, it updates the RIFF length headers
//!  after recording has finished.
//!
//! \return None.
//
//*****************************************************************************
void
NHD0216K3ZSetContrast(uint8_t ui8Contrast)
{
	if(ui8Contrast >= 1 && ui8Contrast <= 50)
	{
		NHD0216K3ZSendCommand(NHD0216K3Z_CONTRAST_SET, ui8Contrast);
	}
}


//*****************************************************************************
//
//! Plays back an audio file.
//!
//! \param readFp is the offset to the register to write.
//!
//!  This function records an audio file in Ogg, MP3, or WAV formats.e
//!  If recording in WAV format, it updates the RIFF length headers
//!  after recording has finished.
//!
//! \return None.
//
//*****************************************************************************
void
NHD0216K3ZInit(uint8_t ui8Contrast, uint8_t ui8Brightness)
{
    SysCtlPeripheralEnable(NHD0216K3_UART_SYSCTL_PERIPH);
    SysCtlPeripheralEnable(NHD0216K3_UART_GPIO_SYSCTL_PERIPH);

    HWREG(NHD0216K3_UART_GPIO_PORT_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(NHD0216K3_UART_GPIO_PORT_BASE + GPIO_O_CR) = NHD0216K3_UART_TX;

    GPIOPinConfigure(NHD0216K3_PINCFG_UARTTX);
    GPIOPinTypeUART(NHD0216K3_UART_GPIO_PORT_BASE, NHD0216K3_UART_TX);

    UARTConfigSetExpClk(NHD0216K3_UART_BASE, SysCtlClockGet(), NHD0216K3_BAUDRATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    NHD0216K3ZSendCommand(NHD0216K3Z_BRIGHTNESS_SET, ui8Brightness);
    NHD0216K3ZSendCommand(NHD0216K3Z_CONTRAST_SET, ui8Contrast);
    NHD0216K3ZSendCommand(NHD0216K3Z_CURSOR_BLINK_OFF, -1);
    NHD0216K3ZSendCommand(NHD0216K3Z_CURSOR_HOME, -1);
    NHD0216K3ZSendCommand(NHD0216K3Z_CLEAR_SCREEN, -1);
    //NHD0216K3ZSendCommand(NHD0216K3Z_BAUD_SET, 4);
}


