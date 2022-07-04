/*

  VLSI Solution generic microcontroller example player / recorder for
  VS1003.

  v1.00 2012-11-28 HH  First release

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/pin_map.h"
#include "grlib/grlib.h"
#include "utils/uartstdio.h"
#include "fatfs/ff.h"
#include "VS10xx.h"
#include "../Manager/FileManager.h"
#include "LED.h"
#include "../Msg.h"



//*****************************************************************************
//
// Printf functions used in this file
//
//*****************************************************************************
#define FILE_ID				("VS10")
#define eprintf(fms, ...)	do{	UARTprintf("\033[31;1m"); \
								UARTprintf("[ %s ] ERROR " fms, FILE_ID, __VA_ARGS__); \
								UARTprintf("\033[0m");} while(0)

#define dprintf(fms, ...)	UARTprintf("[ %s ] " fms, FILE_ID, __VA_ARGS__); \


//*****************************************************************************
//
//! \addtogroup dac_api
//! @{
//
//*****************************************************************************
#define FILE_BUFFER_SIZE                    32
#define SDI_MAX_TRANSFER_SIZE               32
#define SDI_END_FILL_BYTES                  2048

//***************************************************
// Peripheral definitions for VS1003 SPI interface
//***************************************************

// SSI port
#define VS1003_SSI_SYSCTL_PERIPH            SYSCTL_PERIPH_SSI1
#define VS1003_SSI_BASE                     SSI1_BASE

// GPIO for SSI pins
#define VS1003_SSI_GPIO_SYSCTL_PERIPH       SYSCTL_PERIPH_GPIOF
#define VS1003_SSI_GPIO_PORT_BASE           GPIO_PORTF_BASE
#define VS1003_SSI_RX                       GPIO_PIN_0
#define VS1003_SSI_TX                       GPIO_PIN_1
#define VS1003_SSI_CLK                      GPIO_PIN_2
#define VS1003_SSI_PINS                     (VS1003_SSI_RX | VS1003_SSI_TX | VS1003_SSI_CLK)
#define VS1003_SSI_INIT_CLOCK				1000000	// SSI1 clock during initialization
#define VS1003_SSI_CLOCK                   	5000000 // SSI1 clock during normal operation

//*****************************************************************************
//
// Defines the GPIO pin configuration macros for the pins that are used for
// the SSI function.
//
//*****************************************************************************
#define VS1003_PINCFG_SSICLK                GPIO_PF2_SSI1CLK
#define VS1003_PINCFG_SSIRX                 GPIO_PF0_SSI1RX
#define VS1003_PINCFG_SSITX                 GPIO_PF1_SSI1TX

// GPIO for xRST, BSYNC, DREQ pins
#define VS1003_CTL_GPIO_PORT_BASE           GPIO_PORTC_BASE
#define VS1003_CTL_GPIO_SYSCTL_PERIPH       SYSCTL_PERIPH_GPIOC
#define VS1003_xRST                         GPIO_PIN_4
#define VS1003_xDCS                         GPIO_PIN_5
#define VS1003_xCS                          GPIO_PIN_7
#define VS1003_DREQ                         GPIO_PIN_6
#define VS1003_DREQ_INT						GPIO_INT_PIN_6
#define VS1003_CTL_PINS                     (VS1003_xRST | VS1003_xDCS | VS1003_xCS)

//*****************************************************************************
// Global variables
//*****************************************************************************
VS10xxSTATE g_eVS10xxState;						// State-Machine STATE variable
char *g_pcFileParh;								// Path of currenr audio file
FIL g_sFile;									// Structure that holds the current file info
uint8_t g_ui8FileBuffer[FILE_BUFFER_SIZE];		// Buffer that stores the audio data


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
VS10xxSciWrite(uint8_t addr, uint16_t data)
{
	//
	// Check for statemachine in PLAY mode
	//
	if(g_eVS10xxState == VS10xx_PLAY)
	{
		//
		// Disbale interrupts for VS1003_DREQ_INT pin
		//
		GPIOIntDisable(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ_INT);

		//
		// Clear any pending interrupts
		//
		GPIOIntClear(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ_INT);
	}

    //
    // Wait until DREQ becomes high state and execution has been compleated
    //
    while (!GPIOPinRead(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ)){}

    //
    // Set xDCS pin to high state
    //
    GPIOPinWrite(VS1003_CTL_GPIO_PORT_BASE, VS1003_xDCS, VS1003_xDCS);

    //
    // Set xCS pin to low state
    //
    GPIOPinWrite(VS1003_CTL_GPIO_PORT_BASE, VS1003_xCS, 0);

    //
    // Wait for any previous SSI operation to finish.
    //
    while(SSIBusy(VS1003_SSI_BASE)){}

    //
    // Send write code
    //
    SSIDataPut(VS1003_SSI_BASE, WRITE_CODE);

    //
    // Wait for any previous SSI operation to finish.
    //
    while(SSIBusy(VS1003_SSI_BASE)){}

    //
    // Send register adress
    //
    SSIDataPut(VS1003_SSI_BASE, addr);

    //
    // Wait for any previous SSI operation to finish.
    //
    while(SSIBusy(VS1003_SSI_BASE)){}

    //
    // Send high byte to VS1003
    //
    SSIDataPut(VS1003_SSI_BASE, (uint8_t) (data >> 8));

    //
    // Wait for any previous SSI operation to finish.
    //
    while(SSIBusy(VS1003_SSI_BASE)){}

    //
    // Send low byte to VS1003
    //
    SSIDataPut(VS1003_SSI_BASE, (uint8_t) data);

    //
    // Wait for any previous SSI operation to finish.
    //
    while(SSIBusy(VS1003_SSI_BASE)){}

    //
    // Wait for DREQ to go high indicating command is complete
    //
    while (!GPIOPinRead(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ)){}

    //
    // Set xCS pin to high state
    //
    GPIOPinWrite(VS1003_CTL_GPIO_PORT_BASE, VS1003_xCS, VS1003_xCS);

	if(g_eVS10xxState == VS10xx_PLAY)
	{
		GPIOIntEnable(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ_INT);
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
uint16_t
VS10xxSciRead(uint8_t addr)
{
    uint32_t ui8ByteHigh;
    uint32_t ui8ByteLow;

    //
    // Read any residual data from the SSI port.  This makes sure the receive
    // FIFOs are empty, so we don't read any unwanted junk.
    //
    while(SSIDataGetNonBlocking(VS1003_SSI_BASE, 0)){}

    //
    // Set xCS pin to low state
    //
    GPIOPinWrite(VS1003_CTL_GPIO_PORT_BASE, VS1003_xCS, 0);

    //
    // Wait for any previous SSI operation to finish.
    //
    while(SSIBusy(VS1003_SSI_BASE)){}

    //
    // Set VS1003 into read mode
    //
    SSIDataPut(VS1003_SSI_BASE, READ_CODE);

    //
    // Flush data read during the write
    //
    SSIDataGet(VS1003_SSI_BASE, 0);

    //
    // Send register address
    //
    SSIDataPut(VS1003_SSI_BASE, addr);

    //
    // Flush data read during the write
    //
    SSIDataGet(VS1003_SSI_BASE, 0);

    //
    // write dummy byte
    //
    SSIDataPut(VS1003_SSI_BASE, 0xFF);

    //
    // Read high byte
    //
    SSIDataGet(VS1003_SSI_BASE, &ui8ByteHigh);

    //
    // write dummy data
    //
    SSIDataPut(VS1003_SSI_BASE, 0xFF);

    //
    // Read low byte
    //
    SSIDataGet(VS1003_SSI_BASE, &ui8ByteLow);

    //
    // Set xCS pin to high state
    //
    GPIOPinWrite(VS1003_CTL_GPIO_PORT_BASE, VS1003_xCS, VS1003_xCS);


   return ( (uint16_t) ((ui8ByteHigh << 8) | ui8ByteLow) );
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
int
VS10xxSdiWrite(const uint8_t *data, uint8_t bytes)
{
    uint8_t ui8i;

    //
    // Check max. number of bytes to be transfered
    //
    if(bytes > 32)
    {
        return -1;  // Error: Too many bytes to transfer
    }

    //
    // Set xCS pin to high state
    //
    GPIOPinWrite(VS1003_CTL_GPIO_PORT_BASE, VS1003_xCS, VS1003_xCS);

    //
    // Set BSYNC pin to low state
    //
    GPIOPinWrite(VS1003_CTL_GPIO_PORT_BASE, VS1003_xDCS, 0);


    //
    // Send data array to VS1003
    //
    for (ui8i = 0; ui8i < bytes; ui8i++)
    {
        //
        // Wait until DREQ becomes high state
        //
        //while (!GPIOPinRead(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ));

        //
        // Wait for any previous SSI operation to finish.
        //
        while(SSIBusy(VS1003_SSI_BASE)){}

        //
        // Send one byte to VS1003
        //
        SSIDataPut(VS1003_SSI_BASE, data[ui8i]);
    }

    //
    // Wait for any previous SSI operation to finish.
    //
    while(SSIBusy(VS1003_SSI_BASE)){}

    //
    // Set BSYNC pin to high state
    //
    GPIOPinWrite(VS1003_CTL_GPIO_PORT_BASE, VS1003_xDCS, VS1003_xDCS);

    return 0;
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
VS10xxSwReset(void)
{
    VS10xxSciWrite(SCI_MODE, SM_SDINEW | SM_RESET);
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
VS10xxVolumeSet(uint8_t ui8Volume)
{

	dprintf("Lautstaerke: %i%%\n", ui8Volume);

	//
	// Check for corrent volume value range
	//
	if(ui8Volume > 100)
	{
		eprintf("Lautstaerke %i%% ausserhalb des erlaubten Bereiches.\n", ui8Volume);
		return;
	}

	//
	// Scale value range fom 0-100 to 0-127
	//
	ui8Volume = (uint8_t) (127-((ui8Volume/100.0)*127));


	uint16_t ui16Volume = (2 * ui8Volume * 256) + (2 * ui8Volume);

	// ui8Vol = pow(10.0, ui8Vol/20.0);

    VS10xxSciWrite(SCI_VOL, ui16Volume); // analog power down mode
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
VS10xxVolumeMute(void)
{
	//
	// analog power down mode
    //
	VS10xxSciWrite(SCI_VOL, VS10XX_MUTE_VAL);
	dprintf("Audio muted.\n", 0);

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
VS10xxFilePathSet(char *cFilePath)
{
	if(g_eVS10xxState == VS10xx_PLAY || g_eVS10xxState == VS10xx_PAUSE)
	{
		return;
	}

	g_pcFileParh = cFilePath;
	dprintf("Audiodateipfad zu %s gesetzt.\n", g_pcFileParh);
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
VS10xxPlay(void)
{
	uint32_t ui32Cnt;

	switch (g_eVS10xxState)
	{
		case VS10xx_STOP:

			if(g_pcFileParh != NULL)
			{
				dprintf("Oeffne Datei %s...", g_pcFileParh);

				//
				// Open file
				//
				if(!FM_FileOpen(g_pcFileParh, &g_sFile) == FR_OK)
				{
					UARTprintf("Error!\n");

					// Return false in case of an file error
					return false;
				}

			OkPrint;
			}
			else
			{
				//
				// Retuen false in case file path is not set NULL
				//
				return false;
			}


			// Reset Playback
			VS10xxSciWrite(SCI_MODE, SM_SDINEW);

			// Resync
			VS10xxSciWrite(SCI_WRAMADDR, 0x1E29);
			VS10xxSciWrite(SCI_WRAM, 0);

			//
			// As explained in datasheet, set twice 0 in REG_DECODETIME to set time back to 0
			//
			VS10xxSciWrite(SCI_DECODE_TIME, 0x00);
			VS10xxSciWrite(SCI_DECODE_TIME, 0x00);

			//
			// Wait until ready for data
			// Return false in case of an time out
			//
		    ui32Cnt = 0;
		    while (!GPIOPinRead(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ))
		    {
		    	if(ui32Cnt >= 100000)
		    	{
		    		return false;
		    	}
		    	ui32Cnt++;
		    }

		    //
		    // Set state machine into PLAY mode
		    //
		    g_eVS10xxState = VS10xx_PLAY;
			dprintf("Datei %s wird abgespielt.\n", g_pcFileParh);

			//
			// Enable interrupts on VS1003_DREQ_INT pin
			//
			GPIOIntClear(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ_INT);
			GPIOIntEnable(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ_INT);

			//
			// Feed the buffer with audio data
			//
			VS10xxBufferFeed();

		break;

		case VS10xx_PAUSE:
			//
			// Set state maschine into PLAY mode
			//
			g_eVS10xxState = VS10xx_PLAY;

			//
			// Enable interrupts on VS1003_DREQ_INT pin
			//
			GPIOIntClear(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ_INT);
			GPIOIntEnable(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ_INT);

			//
			// Feed the buffer with audio data
			//
			VS10xxBufferFeed();

		break;

		default:break;
	}

	return true;
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
VS10xxStop(void)
{
	/*

	Send complete audio file (MP3, WAV, WMA, MIDI) through SDI
	Send 2048 zeros through SDI (honouring DREQ as usual, so 32 bytes at a time)
	Set SM_RESET (bit 2) of register SCI_MODE
	Wait at least 2 microseconds, then until DREQ is high
	Repeat from beginning

	*/


	uint16_t ui16Idx;
	uint8_t ui8Buf[FILE_BUFFER_SIZE] = {0};

	//
	// Disbale interrupts for VS1003_DREQ_INT pin
	//
	GPIOIntDisable(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ_INT);

	//
	// Clear any pending interrupts
	//
	GPIOIntClear(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ_INT);

	//
	// Cancel all playback by writing lots of zeros
	//
	for (ui16Idx = 0; ui16Idx < SDI_END_FILL_BYTES; ui16Idx += FILE_BUFFER_SIZE)
	{
		//
		// Wait until ready for data
		//
		while(!GPIOPinRead(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ));

		//
		// Send zeros
		//
		VS10xxSdiWrite(ui8Buf, FILE_BUFFER_SIZE);
	}

	//
	// VS10xx software reset
	//
	VS10xxSwReset();

	//
	// Wait at least 2 microseconds
	//
	SysCtlDelay(1000);

	//
	// Wait until ready
	//
	while(!GPIOPinRead(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ));


	dprintf("Wiedegabe von %s gestoppet.\n", g_pcFileParh);

	//
	// Close file
	//
	FM_FileClose(&g_sFile);

    //
    // Set state machine into STOP mode
    //
    g_eVS10xxState = VS10xx_STOP;

	//
	// Disable activity LED
	//
	LEDDisable(LED_ACK);
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
VS10xxPause(void)
{
	//
	// Disable interrupts for VS1003_DREQ_INT pin
	//
	GPIOIntDisable(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ_INT);

	//
	// Clear any pending interrupts
	//
	GPIOIntClear(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ_INT);

    //
    // Set state machine into mode STOP
    //
    g_eVS10xxState = VS10xx_PAUSE;

	//
	// Disable activity LED
	//
	LEDDisable(LED_ACK);

	dprintf("Wiedegabe von %s angehalten.\n", g_pcFileParh);
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
VS10xxBufferFeed(void)
{

	uint32_t ui32BytesRead;
	FRESULT iFResult;

	//
	// Check if in PLAY state
	//
	if(g_eVS10xxState != VS10xx_PLAY)
	{
		return false;
	}

	//
	// Check if ready for data
	//
	if(!GPIOPinRead(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ))
	{
		return false;
	}

	//
	// Feed the buffer until it's full
	//
	while(GPIOPinRead(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ))
	{

		//
		// Read the first/next chunck of data
		//
		iFResult = f_read(&g_sFile, g_ui8FileBuffer, FILE_BUFFER_SIZE, (UINT *) &ui32BytesRead);

		//
		// Check for any error while reading file
		//
		if(iFResult != FR_OK)
		{
			return false;
		}

		//
		// Check for end of file
		//
	    if(ui32BytesRead <= 0)
	    {
	    	GPIOIntDisable(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ_INT);
	    	dprintf("Wiedergabe von Datei %s beebdet.\n", g_pcFileParh);

			//
			// Reset state maschine and file path
			//
	    	g_eVS10xxState = VS10xx_STOP;
	    	g_pcFileParh = NULL;

			//
			// Disable activity LED
			//
	    	LEDDisable(LED_ACK);

	        return false;
	    }

	    //
	    // Transmit data to VS10xx
	    //
	    VS10xxSdiWrite(g_ui8FileBuffer, ui32BytesRead);
	}

	//
	// Return true in case of success
	//
	return true;
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
VS1003IntHandler(void)
{
	//
	// Clear interrupt flag
	//
	GPIOIntClear(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ_INT);

	//
	// Feed the buffer
	//
	VS10xxBufferFeed();

	//
	// Toggle activity LED
	//
	LEDToggle(LED_ACK);
}



VS10xxSTATE
VS10xxStateGet(void)
{
	return g_eVS10xxState;
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
uint8_t
VS10xxInit(void)
{

	dprintf("Initialisiere VS10xx Audiocodec ...", 0);
	//
	// Reset file path
	//
	g_pcFileParh = NULL;

	//
	// Initialize state machine
	//
	g_eVS10xxState = VS10xx_STOP;

	//
	// Enable the peripherals used by this driver
	//
	SysCtlPeripheralEnable(VS1003_SSI_SYSCTL_PERIPH);
	SysCtlPeripheralEnable(VS1003_SSI_GPIO_SYSCTL_PERIPH);
	SysCtlPeripheralEnable(VS1003_CTL_GPIO_SYSCTL_PERIPH);

	//
	// Unlock PF0 so we can change it to SSI function
	// Once we have enabled (unlocked) the commit register then re-lock it
	// to prevent further changes.  PF0 is muxed with NMI thus a special case.
	//
	HWREG(VS1003_SSI_GPIO_PORT_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(VS1003_SSI_GPIO_PORT_BASE + GPIO_O_CR) |= VS1003_SSI_RX;
	HWREG(VS1003_SSI_GPIO_PORT_BASE + GPIO_O_LOCK) = 0;

	//
	// Select the SSI function for the pins used
	//
	GPIOPinConfigure(VS1003_PINCFG_SSITX);
	GPIOPinConfigure(VS1003_PINCFG_SSICLK);

	//
	// Now modify the configuration of the VS1003_PINCFG_SSIRX pin that we unlocked.
	//
	GPIOPinConfigure(VS1003_PINCFG_SSIRX);

	//
	// Configure the pins for the SSI function
	//
	GPIOPinTypeSSI(VS1003_SSI_GPIO_PORT_BASE, VS1003_SSI_PINS);

	//
	// Configure VS1003 xRST ,xDCS and xCS control pins as GPIO output
	//
	GPIOPinTypeGPIOOutput(VS1003_CTL_GPIO_PORT_BASE, VS1003_CTL_PINS);

	//
	// Configure VS1003 DREQ control pins as GPIO input
	//
	GPIOPinTypeGPIOInput(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ);

	//
	// Configure VS1003 DREQ pin interrupt handler
	//
	GPIOIntTypeSet(VS1003_CTL_GPIO_PORT_BASE, VS1003_DREQ, GPIO_RISING_EDGE);
	GPIOIntRegister(VS1003_CTL_GPIO_PORT_BASE, VS1003IntHandler);

	//
	// Disable VS1003
	//
	GPIOPinWrite(VS1003_CTL_GPIO_PORT_BASE, VS1003_xRST, 0);

	//
	// Configure the SSI port
	//
	SSIDisable(VS1003_SSI_BASE);
	SSIConfigSetExpClk( VS1003_SSI_BASE,
						SysCtlClockGet(),
						SSI_FRF_MOTO_MODE_3,
						SSI_MODE_MASTER,
						VS1003_SSI_INIT_CLOCK,
						8);

	SSIEnable(VS1003_SSI_BASE);

	//
	// Enable VS1003
	//
	GPIOPinWrite(VS1003_CTL_GPIO_PORT_BASE, VS1003_xRST, VS1003_xRST);

	//
	// Wait some time before accessing VS10xx
	//
	SysCtlDelay(100000);

	// Software reset
	VS10xxSciWrite(SCI_MODE, SM_SDINEW | SM_RESET);

	// A quick sanity check: write to two registers, then test if we
	// get the same results. Note that if you use a too high SPI
	// speed, the MSB is the most likely to fail when read again.
	VS10xxSciWrite(SCI_HDAT0, 0xABAD);
	VS10xxSciWrite(SCI_HDAT1, 0x1DEA);

	if (VS10xxSciRead(SCI_HDAT0) != 0xABAD || VS10xxSciRead(SCI_HDAT1) != 0x1DEA)
	{
		eprintf("Kommunikationsproblem mit VS10xx Soundkarte.\n", 0);
		return 1;
	}

	//
	// Set the clock. Until this point we need to run SPI slow so that
	// we do not exceed the maximum speeds mentioned in
	// Chapter SPI Timing Diagram in the Datasheet.
	//
	VS10xxSciWrite(SCI_CLOCKF, HZ_TO_SC_FREQ(12288000) | SC_MULT_03_30X | SC_ADD_03_10X);

	//
	// Configure the SSI port
	//
	SSIDisable(VS1003_SSI_BASE);

	SSIConfigSetExpClk( VS1003_SSI_BASE,
						SysCtlClockGet(),
						SSI_FRF_MOTO_MODE_3,
						SSI_MODE_MASTER,
						VS1003_SSI_CLOCK,
						8);

	SSIEnable(VS1003_SSI_BASE);

	OkPrint;

	return 0;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
