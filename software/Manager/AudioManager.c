/*
 *
 * AudioManager.c
 *
 *  Created on: 07.12.2015
 *      Author: Local
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/pin_map.h"
#include "utils/uartstdio.h"
#include "../Driver/fatfs/ff.h"
#include "../Driver/VS10xx.h"
#include "../Driver/Timer.h"
#include "FileManager.h"
#include "AudioManager.h"
#include "../Msg.h"
#include "../System.h"


//*****************************************************************************
//
//! \addtogroup AudioManager_api
//! @{
//
//*****************************************************************************




//*****************************************************************************
//
// Printf functions used in this file
//
//*****************************************************************************
#define FILE_ID				("AUMG")
#define eprintf(...)		do{	UARTprintf("\033[31;1m"); \
								UARTprintf("[ %s ] ERROR " __VA_ARGS__, FILE_ID); \
								UARTprintf("\033[0m");} while(0)

#define dprintf(...)		UARTprintf("[ %s ] " __VA_ARGS__, FILE_ID); \



//*****************************************************************************
// Gobal variables
//******************************************************************************
AMFADE 		g_eFadeType;
AMTRACK 	g_sTrack[N_TRACK];
uint8_t 	g_ui8VolumeLowerLimit = 40;
uint8_t 	ui8VolumeUpperLimit;
uint8_t 	g_ui8CurrentVolume;
uint32_t	g_ui32TrackCnt;
uint8_t		g_ui8DefaultFadeTime;
bool		g_bFirstTrack;
uint16_t	g_ui16NumOfFiles;
bool		g_bVolumFadeRunning;
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
AM_VolumeFade(uint8_t ui8Duration, AMFADE eType)
{
	uint8_t ui8VolumeRange;
	uint16_t ui16TimeSteps;


	if( (VS10xxStateGet() == VS10xx_STOP) || (VS10xxStateGet() == VS10xx_PAUSE) || (true == g_bVolumFadeRunning) )
	{
		// eprintf("Momentan wird keine Musik abgespielt\n");
		return;
	}

	g_eFadeType = eType;

	ui8VolumeRange = ui8VolumeUpperLimit - g_ui8VolumeLowerLimit;
	ui16TimeSteps = (ui8Duration * 1000) /  ui8VolumeRange;
	uint32_t ui32TimerVal = (ui16TimeSteps / 1000.0) * SysCtlClockGet();

	Timer0ALoad(ui32TimerVal);

	switch(g_eFadeType)
	{
		case AM_FADEIN: g_ui8CurrentVolume = g_ui8VolumeLowerLimit; dprintf("Musik wird eingeblendet ...\n"); VS10xxVolumeSet(g_ui8CurrentVolume++); break;
		case AM_FADEOUT: g_ui8CurrentVolume = ui8VolumeUpperLimit; dprintf("Musik wird ausgeblendet ...\n"); VS10xxVolumeSet(g_ui8CurrentVolume--); break;
	}

	//
	// Start fade task
	//
	Timer0AStart();

	g_bVolumFadeRunning = true;

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
AM_VolumeFadeInOutTickHandler(void)
{
	//
	// Clear timer interrupt
	//
	Timer0AIntClear();

	switch(g_eFadeType)
	{
		case AM_FADEIN:

			VS10xxVolumeSet(g_ui8CurrentVolume);
			g_ui8CurrentVolume++;

		    if(g_ui8CurrentVolume > ui8VolumeUpperLimit)
		    {
		    	Timer0AStop();
		    	g_bVolumFadeRunning = false;
		    	dprintf("Einblenden beendet.\n")
		    }

		break;

		case AM_FADEOUT:

			VS10xxVolumeSet(g_ui8CurrentVolume);
			g_ui8CurrentVolume--;

		    if(g_ui8CurrentVolume < g_ui8VolumeLowerLimit)
		    {
		    	Timer0AStop();
		    	AM_Stop();
		    	g_bVolumFadeRunning = false;
		    	dprintf("Ausblenden beendet.\n")
		    }

		break;
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
AM_Play(void)
{
	//
	// Check for number of audio files
	//
	if(g_ui16NumOfFiles == 0)
	{
		eprintf("Keine Audiodaten vorhanden.\n");
		return;
	}

	//
	// Check if audio playback is stopped
	//
	if(VS10xxStateGet() == VS10xx_STOP)
	{
		VS10xxFilePathSet(g_pcFileList[g_ui32TrackCnt++ % g_ui16NumOfFiles]);
		VS10xxPlay();

		if(!g_bFirstTrack)
		{
			AM_VolumeFade(g_sSystem.ui8DefaultFadeTime, AM_FADEIN);
			g_bFirstTrack = true;
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
AM_Stop(void)
{
	//
	// Check if audio playback is stopped
	//
	if(VS10xxStateGet() == VS10xx_PLAY || VS10xxStateGet() == VS10xx_PAUSE)
	{
		VS10xxStop();
		g_bFirstTrack = false;
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
AM_VolumeSet(uint8_t ui8Volume)
{
	ui8VolumeUpperLimit = ui8Volume;
	VS10xxVolumeSet(ui8Volume);
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
AM_Init(uint8_t ui8DefaultVolume, uint8_t ui8DefaultFadeTime, uint16_t ui16NumOfFiles)
{
	//
	// Reset playback flag
	//
	g_bFirstTrack = false;

	//
	// Initialize track counter
	//
	g_ui32TrackCnt = 0;

	//
	// Init default fade time
	//
	g_ui8DefaultFadeTime = ui8DefaultFadeTime;

	//
	// Init number of audio files found by the indexer
	//
	g_ui16NumOfFiles = ui16NumOfFiles;

	//
	// Set initial Volume
	//
	AM_VolumeSet(ui8DefaultVolume);

	//
	// Connect AM_VolumeFadeInOutTickHandler to Timer-0A interrupt
	//
	Timer0AIntConnecte(AM_VolumeFadeInOutTickHandler);
}


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
