/*
 * TaskManager.c
 *
 *  Created on: 27.10.2016
 *      Author: Local
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "driverlib/sysctl.h"
#include "TaskManager.h"
#include "../Msg.h"
#include "../System.h"


//*****************************************************************************
//
// Printf functions used in this file
//
//*****************************************************************************
#define FILE_ID				("TAMG")
#define eprintf(fms, ...)	do{	UARTprintf("\033[31;1m"); \
								UARTprintf("[ %s ] ERROR " fms, FILE_ID, __VA_ARGS__); \
								UARTprintf("\033[0m");} while(0)

#define dprintf(fms, ...)	UARTprintf("[ %s ] " fms, FILE_ID, __VA_ARGS__); \

//*****************************************************************************
//
// Globals
//
//*****************************************************************************
TASK g_sTask[N_TASK];
// TIME g_sDefaultActiveTime;

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
TM_TaskAdd(	TIME	sStartTime,
			TIME	sStopTime,
			TIME	sActiveTime,
			bool	bMotion,
			bool	bAudio,
			bool	bActuator,
			uint8_t	ui8ActuatorProg)
{

	uint8_t ui8Idx;

	for(ui8Idx = 0; ui8Idx < N_TASK; ui8Idx++)
	{
		if(g_sTask[ui8Idx].i16Id == -1)
		{
			dprintf("Erstelle neune Aufgabe ... ", 0);

			//
			// Initialize task index to ui8Idx (position in task list)
			//
			g_sTask[ui8Idx].i16Id = ui8Idx;

			//
			// Set TIME structure to 0
			//

			g_sTask[ui8Idx].sStartTime = sStartTime;
			g_sTask[ui8Idx].sStopTime = sStopTime;
			g_sTask[ui8Idx].sActiveTime = g_sSystem.sDefaultActiveTime;

			//
			// Set motion sensing
			//
			g_sTask[ui8Idx].bMotion = bMotion;

			//
			// Set audio
			//
			g_sTask[ui8Idx].bAudio = bAudio;

			//
			// Set 230VAC actuators
			//
			g_sTask[ui8Idx].bActuator = bActuator;

			//
			// Set 230VAC actuators program
			//
			g_sTask[ui8Idx].ui8ActuatorProg = ui8ActuatorProg;


			OkPrint;

			return 1;
		}

	}

	eprintf("Maximale Aufgabenanzahl (%i) erreicht.\n", N_TASK);

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
TM_TaskRemove(uint8_t ui8TaskIdxToRemove)
{

	if((g_sTask[ui8TaskIdxToRemove - 1].i16Id == -1) || (ui8TaskIdxToRemove > N_TASK) || (ui8TaskIdxToRemove <= 0)   )
	{
		eprintf(" Aufgabe %i nicht vorhanden\n ", ui8TaskIdxToRemove);
		return;
	}

	dprintf("Lösche Aufgabe %i ... ", ui8TaskIdxToRemove);

	//
	// Initialize task index to ui8TaskIdxToRemove (position in task list)
	//
	g_sTask[ui8TaskIdxToRemove - 1].i16Id = -1;

	//
	// Set TIME structure to 0
	//
	memset(&g_sTask[ui8TaskIdxToRemove - 1].sStartTime, -1, sizeof(TIME));
	memset(&g_sTask[ui8TaskIdxToRemove - 1].sStopTime, -1, sizeof(TIME));
	memset(&g_sTask[ui8TaskIdxToRemove - 1].sActiveTime, -1, sizeof(TIME));

	//
	// Disable audio
	//
	g_sTask[ui8TaskIdxToRemove - 1].bAudio = false;

	//
	// Disable motion sensing
	//
	g_sTask[ui8TaskIdxToRemove - 1].bMotion = false;

	//
	// Disable 230VAC actuators
	//
	g_sTask[ui8TaskIdxToRemove - 1].bActuator = false;

	OkPrint;
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
TM_TaskSet(uint8_t ui8TaskIdxToSet, TASKPARAM eParam, uint8_t ui8Value)
{
	if((g_sTask[ui8TaskIdxToSet - 1].i16Id == -1) || (ui8TaskIdxToSet > N_TASK) || (ui8TaskIdxToSet <= 0)   )
	{
		eprintf(" Aufgabe %i nicht vorhanden.\n ", ui8TaskIdxToSet);
		return;
	}

	switch (eParam)
	{
		case STARTTIME_H:	g_sTask[ui8TaskIdxToSet - 1].sStartTime.i8Hour = ui8Value;		break;
		case STARTTIME_M:	g_sTask[ui8TaskIdxToSet - 1].sStartTime.i8Minute = ui8Value;	break;
		case STARTTIME_S:	g_sTask[ui8TaskIdxToSet - 1].sStartTime.i8Second = ui8Value;	break;
		case STOPTIME_H:	g_sTask[ui8TaskIdxToSet - 1].sStopTime.i8Hour = ui8Value;		break;
		case STOPTIME_M:	g_sTask[ui8TaskIdxToSet - 1].sStopTime.i8Minute = ui8Value;		break;
		case STOPTIME_S:	g_sTask[ui8TaskIdxToSet - 1].sStopTime.i8Second = ui8Value;		break;
		case ACTIVETIME_H:	g_sTask[ui8TaskIdxToSet - 1].sActiveTime.i8Hour = ui8Value;		break;
		case ACTIVETIME_M:	g_sTask[ui8TaskIdxToSet - 1].sActiveTime.i8Minute = ui8Value;	break;
		case ACTIVETIME_S:	g_sTask[ui8TaskIdxToSet - 1].sActiveTime.i8Second = ui8Value;	break;
		case MOTION:		g_sTask[ui8TaskIdxToSet - 1].bMotion = ui8Value;				break;
		case AUDIO:			g_sTask[ui8TaskIdxToSet - 1].bAudio = ui8Value;					break;
		case ACTUATOR:		g_sTask[ui8TaskIdxToSet - 1].bActuator = ui8Value;				break;
		case ACTUATOR_CH:

			if(g_sTask[ui8TaskIdxToSet - 1].bActuatorCH[ui8Value - 1])
			{
				g_sTask[ui8TaskIdxToSet - 1].bActuatorCH[ui8Value - 1] = false;
			}
			else
			{
				g_sTask[ui8TaskIdxToSet - 1].bActuatorCH[ui8Value - 1] = true;
			}

			break;

		case ACTUATOR_PROG:	g_sTask[ui8TaskIdxToSet - 1].ui8ActuatorProg = ui8Value;		break;
	}

	//
	// Calculate time in seconds
	//
	g_sTask[ui8TaskIdxToSet - 1].sStartTime.i32TimeInSeconds = TIME2SECONDS(g_sTask[ui8TaskIdxToSet - 1].sStartTime.i8Hour, g_sTask[ui8TaskIdxToSet - 1].sStartTime.i8Minute, g_sTask[ui8TaskIdxToSet - 1].sStartTime.i8Second);
	g_sTask[ui8TaskIdxToSet - 1].sStopTime.i32TimeInSeconds = TIME2SECONDS(g_sTask[ui8TaskIdxToSet - 1].sStopTime.i8Hour, g_sTask[ui8TaskIdxToSet - 1].sStopTime.i8Minute, g_sTask[ui8TaskIdxToSet - 1].sStopTime.i8Second);
	g_sTask[ui8TaskIdxToSet - 1].sActiveTime.i32TimeInSeconds = TIME2SECONDS(g_sTask[ui8TaskIdxToSet - 1].sActiveTime.i8Hour, g_sTask[ui8TaskIdxToSet - 1].sActiveTime.i8Minute, g_sTask[ui8TaskIdxToSet - 1].sActiveTime.i8Second);

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
TM_TaskPrint(void)
{

	uint8_t ui8Idx;
	uint8_t ui8Cnt;

	UARTprintf("\n+-----------------------------------------------------------------------------------------------------------------------------------+\n");
	  UARTprintf("|                                                            AUFGABENLISTE                                                          |\n");
	  UARTprintf("+-----------------------------------------------------------------------------------------------------------------------------------+\n");
	  UARTprintf("|   Nr.  |  Id  |  Startzeit  |  Stopzeit  |  Aktivzeit  |  Bewegung  |  Audio  |  Aktor  | Ch 1 | Ch 2 | Ch 3 | Ch 4 | Ch 5 | Ch 6 |\n");
	  UARTprintf("+-----------------------------------------------------------------------------------------------------------------------------------+\n");

	ui8Cnt = 0;
	for(ui8Idx = 0; ui8Idx < N_TASK; ui8Idx++)
	{
		if(g_sTask[ui8Idx].i16Id != -1)
		{
			SysCtlDelay(500000);

			UARTprintf("|   %02i   |  %02i  |    %02i:%02i    |    %02i:%02i   |    %02i:%02i    |     %c      |    %c    |    %c    |   %c  |   %c  |   %c  |   %c  |   %c  |   %c  |\n",
						++ui8Cnt,
						ui8Idx + 1,
						g_sTask[ui8Idx].sStartTime.i8Hour,
						g_sTask[ui8Idx].sStartTime.i8Minute,
						g_sTask[ui8Idx].sStopTime.i8Hour,
						g_sTask[ui8Idx].sStopTime.i8Minute,
						g_sTask[ui8Idx].sActiveTime.i8Minute,
						g_sTask[ui8Idx].sActiveTime.i8Second,
						(g_sTask[ui8Idx].bMotion) ? 'X' : '-',
						(g_sTask[ui8Idx].bAudio) ? 'X' : '-',
						(g_sTask[ui8Idx].bActuator) ? 'X' : '-',
						(g_sTask[ui8Idx].bActuatorCH[0]) ? 'X' : '-',
						(g_sTask[ui8Idx].bActuatorCH[1]) ? 'X' : '-',
						(g_sTask[ui8Idx].bActuatorCH[2]) ? 'X' : '-',
						(g_sTask[ui8Idx].bActuatorCH[3]) ? 'X' : '-',
						(g_sTask[ui8Idx].bActuatorCH[4]) ? 'X' : '-',
						(g_sTask[ui8Idx].bActuatorCH[5]) ? 'X' : '-');
		}
	}

	UARTprintf("+-----------------------------------------------------------------------------------------------------------------------------------+\n");
}


//*****************************************************************************
//
//! Search task list for matching entries
//!
//! \param i32CurrentTimeInSeconds Current time in seconds
//!
//!  This function will write the register passed in ucAddr with the value
//!  passed in to ulData.  The data in ulData is actually 9 bits and the
//!  value in ucAddr is interpreted as 7 bits.
//!
//! \return True on success or false on error.
//
//*****************************************************************************
int8_t
TM_TaskSearch(int32_t i32CurrentTimeInSeconds)
{

	uint8_t ui8Idx;
	int8_t	i8Result = -1;

	//
	// Search in task list for matching task entries
	//
	for(ui8Idx = 0; ui8Idx < N_TASK; ui8Idx++)
	{
		if(g_sTask[ui8Idx].sStartTime.i32TimeInSeconds >= 0)
		{
			if( (i32CurrentTimeInSeconds > g_sTask[ui8Idx].sStartTime.i32TimeInSeconds) && (i32CurrentTimeInSeconds < g_sTask[ui8Idx].sStopTime.i32TimeInSeconds) )
			{

				// dprintf("Aufgabe %i aktiviert, %02i:%02i:%02i - %02i:%02i:%02i\n", ui8Idx + 1, g_sTask[ui8Idx].sStartTime.i8Hour, g_sTask[ui8Idx].sStartTime.i8Minute, g_sTask[ui8Idx].sStartTime.i8Second, g_sTask[ui8Idx].sStopTime.i8Hour, g_sTask[ui8Idx].sStopTime.i8Minute, g_sTask[ui8Idx].sStopTime.i8Second);

				//
				// Store task index
				//
				i8Result = ui8Idx;

				//
				// Exit loop
				//
				break;
			}
		}
	}

	return i8Result;
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
TM_Init()
{

	uint8_t ui8Idx;

	//
	// Set default task active time
	//
	//g_sDefaultActiveTime = sDefaultActiveTime;

	//
	// Initialize task structure
	//
	for(ui8Idx = 0; ui8Idx < N_TASK; ui8Idx++)
	{
		//
		// Reset task index (position in task list)
		//
		g_sTask[ui8Idx].i16Id = -1;

		//
		// Initialize TIME structure to -1
		//
		memset(&g_sTask[ui8Idx].sStartTime, -1, sizeof(TIME));
		memset(&g_sTask[ui8Idx].sStopTime, -1, sizeof(TIME));
		memset(&g_sTask[ui8Idx].sActiveTime, -1, sizeof(TIME));

		//
		// Disable audio
		//
		g_sTask[ui8Idx].bAudio = false;

		//
		// Disable motion sensing
		//
		g_sTask[ui8Idx].bMotion = false;

		//
		// Disable 230VAC actuators
		//
		g_sTask[ui8Idx].bActuator = false;

		//
		// Disable all actuator channels
		//
		memset(&g_sTask[ui8Idx].bActuatorCH, 0, sizeof(uint8_t) * ACT_N_CH);
	}
}
