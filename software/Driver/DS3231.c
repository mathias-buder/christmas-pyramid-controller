/*
 * DS3231.c
 *
 *  Created on: 12.08.2015
 *      Author: Mathias
 */
#include <string.h>
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
#include "utils/uartstdio.h"
#include "DS3231.h"
#include "LED.h"

//*****************************************************************************
//
//! \addtogroup DS3231_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// The I2C interface used by this driver.
//
//*****************************************************************************
#define DS3231_I2C_PERIPH               SYSCTL_PERIPH_I2C1
#define DS3231_I2C_GPIO_PERIPH          SYSCTL_PERIPH_GPIOA
#define DS3231_I2CSCL_GPIO              GPIO_PA6_I2C1SCL
#define DS3231_I2CSDA_GPIO              GPIO_PA7_I2C1SDA
#define DS3231_I2C_BASE                 GPIO_PORTA_BASE
#define DS3231_I2CSDA_PIN               GPIO_PIN_7
#define DS3231_I2CSCL_PIN				GPIO_PIN_6
#define DS3231_I2C_MASTER_BASE			I2C1_BASE

//*****************************************************************************
//
// GPIO pins used by this driver
//
//*****************************************************************************
#define DS3231_GPIO_SYSCTL_PERIPH		SYSCTL_PERIPH_GPIOE
#define DS3231_GPIO_PORT_BASE			GPIO_PORTE_BASE
#define DS3231_PIN						GPIO_PIN_5
#define DS3231_INT_PIN					GPIO_INT_PIN_5

//*****************************************************************************
//
// Globals
//
//*****************************************************************************
uint8_t g_ui8DS3231State;
float g_fTemperature = 0.0;
DS3231 g_sDS3231Data = {0,0,0,0,0,0,0};


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
DS3231RegisterWriteSingle(uint8_t ui8Register, uint8_t pui8Data)
{
    //
    // Set the slave address.
    //
	I2CMasterSlaveAddrSet(DS3231_I2C_MASTER_BASE, DS3231_I2C_ADDRESS, false);

    //
    // Write the first byte to the controller (register)
    //
    I2CMasterDataPut(DS3231_I2C_MASTER_BASE, ui8Register);

    //
    // Continue the transfer.
    //
    I2CMasterControl(DS3231_I2C_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    //
    // Wait until the current byte has been transferred.
    //
    while(I2CMasterBusy(DS3231_I2C_MASTER_BASE));


    if(I2CMasterErr(DS3231_I2C_MASTER_BASE) != I2C_MASTER_ERR_NONE)
    {
        I2CMasterIntClear(DS3231_I2C_MASTER_BASE);
        return(false);
    }

	//
	// Write the data byte to the controller.
	//
	I2CMasterDataPut(DS3231_I2C_MASTER_BASE, pui8Data);

    //
    // End the transfer.
    //
    I2CMasterControl(DS3231_I2C_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

	//
	// Wait until the current byte has been transferred.
	//
	while(I2CMasterBusy(DS3231_I2C_MASTER_BASE));


    if(I2CMasterErr(DS3231_I2C_MASTER_BASE) != I2C_MASTER_ERR_NONE)
    {
        return(false);
    }

    return(true);
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
DS3231RegisterSetSingle(uint8_t ui8Register, uint8_t ui8Data)
{
    uint8_t ui8DataCheck;

    //
    // Send date/time via I2C channel
    //
    if(!DS3231RegisterWriteSingle(ui8Register, ui8Data))
    {
        return(false);
    }

    //
    // Check that data has been saved succsessfully
    //
    // Read register that just has been set
    //
    if(!DS3231RegisterReadSingle(ui8Register, &ui8DataCheck))
    {
        return(false);
    }

    //
    // Check send value againsed received value
    //
    if(ui8Data != ui8DataCheck)
    {
        return(false);
    }

    return(true);
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
DS3231RegisterWriteBurst(uint8_t ui8Register, uint8_t *pui8Data, uint8_t ui8Size)
{
	uint8_t ui8Loop;

    //
    // Set the slave address.
    //
    I2CMasterSlaveAddrSet(DS3231_I2C_MASTER_BASE, DS3231_I2C_ADDRESS, false);

    //
    // Write the first byte to the controller (register)
    //
    I2CMasterDataPut(DS3231_I2C_MASTER_BASE, ui8Register);

    //
    // Continue the transfer.
    //
    I2CMasterControl(DS3231_I2C_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    //
    // Wait until the current byte has been transferred.
    //
    while(I2CMasterBusy(DS3231_I2C_MASTER_BASE));


    if(I2CMasterErr(DS3231_I2C_MASTER_BASE) != I2C_MASTER_ERR_NONE)
    {
        I2CMasterIntClear(DS3231_I2C_MASTER_BASE);
        return(false);
    }

    //
    // Send all data
    //
    for(ui8Loop = 0; ui8Loop < ui8Size; ui8Loop++)
    {
        //
        // Write the data byte to the controller.
        //
        I2CMasterDataPut(DS3231_I2C_MASTER_BASE, pui8Data[ui8Loop]);

        //
		// Continue the transfer.
		//
		I2CMasterControl(DS3231_I2C_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

        //
        // Wait until the current byte has been transferred.
        //
        while(I2CMasterBusy(DS3231_I2C_MASTER_BASE));
    }

    //
    // End the transfer.
    //
    I2CMasterControl(DS3231_I2C_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

    //
    // Wait until the current byte has been transferred.
    //
    while(I2CMasterBusy(DS3231_I2C_MASTER_BASE));

    if(I2CMasterErr(DS3231_I2C_MASTER_BASE) != I2C_MASTER_ERR_NONE)
    {
        return(false);
    }

    return(true);
}

//*****************************************************************************
//
// Reads a register in the the DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
bool
DS3231RegisterReadSingle(uint8_t ui8Register, uint8_t *pui8Data)
{
    //
    // Set the slave address and "WRITE"
    //
    I2CMasterSlaveAddrSet(DS3231_I2C_MASTER_BASE, DS3231_I2C_ADDRESS, false);

    //
    // Write the first byte to the controller (register)
    //
    I2CMasterDataPut(DS3231_I2C_MASTER_BASE, ui8Register);

    //
    // Continue the transfer.
    //
    I2CMasterControl(DS3231_I2C_MASTER_BASE, I2C_MASTER_CMD_SINGLE_SEND);

    //
    // Wait until the current byte has been transferred.
    //
    while(I2CMasterBusy(DS3231_I2C_MASTER_BASE));

    //
    // Check for any errors during transmission
    //
    if(I2CMasterErr(DS3231_I2C_MASTER_BASE) != I2C_MASTER_ERR_NONE)
    {
        I2CMasterIntClear(DS3231_I2C_MASTER_BASE);
        return(false);
    }

    //
    // Set the slave address and "READ"/true.
    //
    I2CMasterSlaveAddrSet(DS3231_I2C_MASTER_BASE, DS3231_I2C_ADDRESS, true);

    //
    // Read Data Byte.
    //
    I2CMasterControl(DS3231_I2C_MASTER_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

    //
    // Wait until the current byte has been transferred.
    //
    while(I2CMasterBusy(DS3231_I2C_MASTER_BASE));

    //
    // Check for error during transmission
    //
    if(I2CMasterErr(DS3231_I2C_MASTER_BASE) != I2C_MASTER_ERR_NONE)
    {
        I2CMasterIntClear(DS3231_I2C_MASTER_BASE);
        return(false);
    }

    //
    // Read the value received.
    //
    *pui8Data  = I2CMasterDataGet(DS3231_I2C_MASTER_BASE);

    return(true);
}

//*****************************************************************************
//
// Reads a register in the the DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
bool
DS3231RegisterReadBurst(uint8_t ui8Register, uint8_t *pui8Data, uint8_t ui8Size)
{
	uint8_t ui8Loop;

    //
    // Set the slave address and "WRITE"
    //
    I2CMasterSlaveAddrSet(DS3231_I2C_MASTER_BASE, DS3231_I2C_ADDRESS, false);

    //
    // Write the first byte to the controller (register)
    //
    I2CMasterDataPut(DS3231_I2C_MASTER_BASE, ui8Register);

    //
    // Continue the transfer.
    //
    I2CMasterControl(DS3231_I2C_MASTER_BASE, I2C_MASTER_CMD_SINGLE_SEND);

    //
    // Wait until the current byte has been transferred.
    //
    while(I2CMasterBusy(DS3231_I2C_MASTER_BASE));

    //
    // Check for errors during transmission
    //
    if(I2CMasterErr(DS3231_I2C_MASTER_BASE) != I2C_MASTER_ERR_NONE)
    {
		I2CMasterIntClear(DS3231_I2C_MASTER_BASE);
        return(false);
    }

    //
    // Set the slave address and "READ"/true.
    //
    I2CMasterSlaveAddrSet(DS3231_I2C_MASTER_BASE, DS3231_I2C_ADDRESS, true);

    //
    // Read Data Byte
    //
    I2CMasterControl(DS3231_I2C_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

    //
    // Wait until the current byte has been transferred.
    //
    while(I2CMasterBusy(DS3231_I2C_MASTER_BASE));

    //
    // Check for errors during transmission.
    //
    if(I2CMasterErr(DS3231_I2C_MASTER_BASE) != I2C_MASTER_ERR_NONE)
    {
		I2CMasterIntClear(DS3231_I2C_MASTER_BASE);
        return(false);
    }

    //
    // Read the value received.
    //
    for(ui8Loop = 0; ui8Loop < ui8Size; ui8Loop++)
    {
    	pui8Data[ui8Loop]  = I2CMasterDataGet(DS3231_I2C_MASTER_BASE);
    	I2CMasterControl(DS3231_I2C_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
    	while(I2CMasterBusy(DS3231_I2C_MASTER_BASE));
    }

    I2CMasterControl(DS3231_I2C_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

    //
    // Wait until the current byte has been transferred.
    //
    while(I2CMasterBusy(DS3231_I2C_MASTER_BASE));


    //
    // Check for errors during transmission.
    //
    if(I2CMasterErr(DS3231_I2C_MASTER_BASE) != I2C_MASTER_ERR_NONE)
    {
        return(false);
    }

    return(true);
}


//*****************************************************************************
//
// Reads a register in the the DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
bool
DS3231RegBitSet(uint8_t ui8Register, uint8_t ui8BitMask)
{
	uint8_t ui8ReceivedData;

	if(!DS3231RegisterReadSingle(ui8Register, &ui8ReceivedData))
	{
		return(false);
	}

	if(!DS3231RegisterWriteSingle(ui8Register, ui8ReceivedData | ui8BitMask))
	{
		return (false);
	}

    return(true);
}



//*****************************************************************************
//
// Reads a register in the the DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
bool
DS3231RegBitReset(uint8_t ui8Register, uint8_t ui8BitMask)
{
	uint8_t ui8ReceivedData;

	if(!DS3231RegisterReadSingle(ui8Register, &ui8ReceivedData))
	{
		return(false);
	}

	if(!DS3231RegisterWriteSingle(ui8Register, ui8ReceivedData & ~ui8BitMask))
	{
		return (false);
	}

    return(true);
}


//*****************************************************************************
//
// Convert normal decimal numbers to binary coded decimal
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
uint8_t
Dec2Bcd(uint8_t ui8Val)
{
  return( (ui8Val/10*16) + (ui8Val%10) );
}

//*****************************************************************************
//
// Convert binary coded decimal to normal decimal numbers
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
uint8_t
Bcd2Dec(uint8_t ui8Val)
{
  return( (ui8Val/16*10) + (ui8Val%16) );
}


//*****************************************************************************
//
// Set time and date in DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
bool
DS3231TimeSet(DS3231* sDate)
{
	uint8_t ui8Idx;

	//
	// Convert values from deciaml to BCD formt and save
	// it into DS3231 data array
	//
	for(ui8Idx = 0; ui8Idx < sizeof(sDate)/sizeof(uint8_t); ui8Idx++)
	{
		((uint8_t*) sDate)[ui8Idx] = Dec2Bcd(((uint8_t*) sDate)[ui8Idx]);
	}

	//
	// Send date/time via I2C channel
	//
	if(!DS3231RegisterWriteBurst(0, (uint8_t*) sDate, sizeof(sDate)/sizeof(uint8_t)))
	{
		return(false);
	}

	return(true);
}

//*****************************************************************************
//
// Set time and date in DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
bool
DS3231DaySet(uint8_t ui8Data)
{
	//
	// Send date/time via I2C channel
	//
	if(!DS3231RegisterSetSingle(DS3231_DAY, Dec2Bcd(ui8Data)))
	{
		return(false);
	}

	return(true);
}

//*****************************************************************************
//
// Set time and date in DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
bool
DS3231DateSet(uint8_t ui8Data)
{
	//
	// Send date/time via I2C channel
	//
	if(!DS3231RegisterSetSingle(DS3231_DATE, Dec2Bcd(ui8Data)))
	{
		return(false);
	}

	return(true);
}

//*****************************************************************************
//
// Set time and date in DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
bool
DS3231MonthSet(uint8_t ui8Data)
{
    //
    // Send date/time via I2C channel
    //
    if(!DS3231RegisterSetSingle(DS3231_MONTH, Dec2Bcd(ui8Data)))
    {
        return(false);
    }

    return(true);
}

//*****************************************************************************
//
// Set time and date in DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
bool
DS3231YearSet(uint8_t ui8Data)
{
    //
    // Send date/time via I2C channel
    //
    if(!DS3231RegisterSetSingle(DS3231_YEAR, Dec2Bcd(ui8Data)))
    {
        return(false);
    }

    return(true);
}

//*****************************************************************************
//
// Set time and date in DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
bool
DS3231HourSet(uint8_t ui8Data)
{
    //
    // Send date/time via I2C channel
    //
    if(!DS3231RegisterSetSingle(DS3231_HOUR, Dec2Bcd(ui8Data)))
    {
        return(false);
    }

    return(true);
}

//*****************************************************************************
//
// Set time and date in DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
bool
DS3231MinuteSet(uint8_t ui8Data)
{
    //
    // Send date/time via I2C channel
    //
    if(!DS3231RegisterSetSingle(DS3231_MINUTES, Dec2Bcd(ui8Data)))
    {
        return(false);
    }

    return(true);
}

//*****************************************************************************
//
// Set time and date in DS3231 RTC.
//
// \param ucRegister is the offset to the register to write.
// \param pucData is a pointer to the returned data.
//
//  \return \b true on success or \b false on error.
//
//*****************************************************************************
bool
DS3231SecondSet(uint8_t ui8Data)
{
    //
    // Send date/time via I2C channel
    //
    if(!DS3231RegisterSetSingle(DS3231_SECONDS, Dec2Bcd(ui8Data)))
    {
        return(false);
    }

    return(true);
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
//*****************************************************************************
DS3231*
DS3231TimeGet(void)
{
	return &g_sDS3231Data;
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
//*****************************************************************************
bool
DS3231TemperatureGet(float *pfData)
{
	uint8_t pui8TempData[DS3231_TEMP_DATA];

	//
	// Trigger temperatur conversion
	//
	if(!DS3231RegBitSet(DS3231_CTL, DS3231_CTL_CONV))
	{
		return(false);
	}

	//
	// Read data over I2C channel
	//
	if(!DS3231RegisterReadBurst(DS3231_TEMP_MSB, pui8TempData, DS3231_TEMP_DATA))
	{
		return(false);
	}

	//
	// Convert Data to float (from 2's complement)
	//
	*pfData = (pui8TempData[0] & 0x7F) + ((pui8TempData[1] >> 6) * 0.25);

	return(true);
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
//*****************************************************************************
bool
DS3231TemperatureOffsetSet(int8_t i8Data)
{
	//
	// Set temperature aging offset
	//
	if(!DS3231RegisterWriteSingle(DS3231_AGING_OFFSET, i8Data))
	{
		return(false);
	}

	return(true);
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
//*****************************************************************************
bool
DS3231AllRegisterGet(uint8_t *ui8Register, uint8_t ui8Size)
{
	//
	// Get entire register content
	//
	if(!DS3231RegisterReadBurst(0x00, ui8Register, ui8Size))
	{
		return(false);
	}

	return(true);
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
//*****************************************************************************
void
DS3231IntHandler(void)
{
	uint8_t ui8Idx;

	//
	// Clear all pending interrupts
	//
	GPIOIntClear(DS3231_GPIO_PORT_BASE, DS3231_INT_PIN);

	//
	// Get time and date
	//
	if(DS3231RegisterReadBurst(0, (uint8_t*) &g_sDS3231Data, sizeof(g_sDS3231Data)/sizeof(uint8_t)))
	{
		//
		// Convert data from BCD to decimal format
		//
		for(ui8Idx = 0; ui8Idx < sizeof(g_sDS3231Data)/sizeof(uint8_t); ui8Idx++)
		{
			((uint8_t*) &g_sDS3231Data)[ui8Idx] = Bcd2Dec(((uint8_t*) &g_sDS3231Data)[ui8Idx]);
		}

		//
		// Calculate current time in seconds
		//
		g_sDS3231Data.i32TimeInSeconds = TIME2SECONDS(g_sDS3231Data.ui8Hour, g_sDS3231Data.ui8Minute, g_sDS3231Data.ui8Second);

		g_ui8DS3231State = DS3231_OK;
	}
	else
	{
		g_ui8DS3231State = DS3231_ERROR;
	}


	//
	//	Print time to console for debug
	//
	//UARTprintf("%02d:%02d:%02d\n", g_sDS3231Data.ui8Hour, g_sDS3231Data.ui8Minute, g_sDS3231Data.ui8Second);
	//UARTprintf("TimeInSeconds: %d\n", g_sDS3231Data.i32TimeInSeconds);
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
bool DS3231Init(void)
{
	//
	// Enable the GPIO port containing the I2C pins and set the SDA pin as a
	// GPIO input for now and engage a weak pull-down.  If the daughter board
	// is present, the pull-up on the board should easily overwhelm
	// the pull-down and we should read the line state as high.
	//

	SysCtlPeripheralEnable(DS3231_I2C_PERIPH);

	//
	// Enable the I2C peripheral.
	//
	SysCtlPeripheralEnable(DS3231_I2C_GPIO_PERIPH);

	//
	// Enable the peripheral used for DS3231 INT/SQW pin
	//
    SysCtlPeripheralEnable(DS3231_GPIO_SYSCTL_PERIPH);

	//
	// Delay a while to ensure that we read a stable value from the SDA
	// GPIO pin.  If we read too quickly, the result is unpredictable.
	// This delay is around 2mS.
	//
	SysCtlDelay(SysCtlClockGet() / (3 * 500));

	//
	// Configure the pin mux.
	//
	GPIOPinConfigure(DS3231_I2CSCL_GPIO);
	GPIOPinConfigure(DS3231_I2CSDA_GPIO);

	//
	// Configure the I2C SCL and SDA pins for I2C operation.
	//
	GPIOPinTypeI2C(DS3231_I2C_BASE, DS3231_I2CSDA_PIN);
	GPIOPinTypeI2CSCL(DS3231_I2C_BASE, DS3231_I2CSCL_PIN);
	//
	// Initialize the I2C master.
	//
	I2CMasterInitExpClk(DS3231_I2C_MASTER_BASE, SysCtlClockGet(), true);

	//
	// Set coresponding pins to be a GPIO input
	//
    GPIOPinTypeGPIOInput(DS3231_GPIO_PORT_BASE, DS3231_PIN);

    //
    // Set interrupt trogger to rising edge
    //
    GPIOIntTypeSet(DS3231_GPIO_PORT_BASE, DS3231_INT_PIN, GPIO_RISING_EDGE);

    //
    // Connect ISR to DS3231 interrupt
    //
    GPIOIntRegister(DS3231_GPIO_PORT_BASE, DS3231IntHandler);

	//
	// Set the INT/SQW signal to react on match between time keeping register
	// and alarm registers of Alarm 1 and Alarm 2
	//
    DS3231RegisterWriteSingle(DS3231_CTL, 0b00000000);

	//
	// Enable interrupts at DS3231_INT_PIN
	//
    GPIOIntEnable(DS3231_GPIO_PORT_BASE, DS3231_INT_PIN);

	//
	// Return true in case everything went fine
	//
	return(true);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************















