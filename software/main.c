//*****************************************************************************
//
// sd_card.c - Example program for reading files from an SD card.
//
// Copyright (c) 2011-2015 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.

// 
// This is part of revision 2.1.1.71 of the DK-TM4C123G Firmware Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "utils/cmdline.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "Driver/ACT.h"
#include "Driver/fatfs/ff.h"
#include "Driver/fatfs/diskio.h"
#include "Driver/VS10xx.h"
#include "Driver/DS3231.h"
#include "Manager/ConsoleManager.h"
#include "Manager/FileManager.h"
#include "Manager/AudioManager.h"
#include "Manager/DisplayManager.h"
#include "Manager/TaskManager.h"
#include "Driver/LED.h"
#include "Driver/NHD0216K3Z.h"
#include "Driver/HCSR501.h"
#include "Driver/KY040.h"
#include "Driver/Timer.h"
#include "Msg.h"
#include "System.h"


//*****************************************************************************
//
// printf functions used in this file
//
//*****************************************************************************
#define eprintf(fms, ...)	do{	UARTprintf("\033[31;1m"); \
								UARTprintf("[ MAIN ] ERROR " fms, __VA_ARGS__); \
								UARTprintf("\033[0m");} while(0)

#define dprintf(fms, ...)	UARTprintf("[ MAIN ] " fms, __VA_ARGS__); \

//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void

__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

#define FW_VERSION				("0.1")

//*****************************************************************************
//
// Defines the size of the buffer that holds the command line.
//
//*****************************************************************************
#define CMD_BUF_SIZE            (64)


//*****************************************************************************
//
// The buffer that holds the command line.
//
//*****************************************************************************
static char g_pcCmdBuf[CMD_BUF_SIZE];

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //

//#ifdef UART_9600
//    UARTStdioConfig(0, 9600, 16000000);
//#else
    UARTStdioConfig(0, 115200, 16000000);
//#endif
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
PortDIntHandler(void)
{
	uint32_t ui32IntStatus;

	//
	// Get current interrupt state
	//
	ui32IntStatus = GPIOIntStatus(GPIO_PORTD_BASE, false);

	//
	// Clear the flag that triggered the interrupt
	//
	GPIOIntClear(GPIO_PORTD_BASE, ui32IntStatus);

	//
	// Call coresponding callback-function
	//
	if(ui32IntStatus & GPIO_INT_PIN_1)
	{
		HCSR501OnMotion();
	}

	if(ui32IntStatus & GPIO_INT_PIN_2)
	{
		FM_SDCheck();
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
ConvertSecondsInHhMmSs(uint32_t ui32TotalSeconds, int8_t* pi8Hour, int8_t* pi8Minutes, int8_t* pi8Seconds)
{
	*pi8Seconds = (ui32TotalSeconds % 60);
	*pi8Minutes = (ui32TotalSeconds % 3600) / 60;
	*pi8Hour = (ui32TotalSeconds % 86400) / 3600;
}


//*****************************************************************************
//
// The program main function.  It performs initialization, then runs a command
// processing loop to read commands from the console.
//
//*****************************************************************************

int
main(void)
{
    int nStatus;
    int8_t i8TaskIdx;
	uint8_t ui8Cnt, bTaskRunning, bSubseqTaskFound;
    TIME sMotionExitTime;
    TIME sEventTimeout;
    uint32_t ui32CurrentTimeInSeconds;
    uint16_t ui16IndexedFiles = 0;

    //
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    FPULazyStackingEnable();

    //
    // Set the system clock to run at 80MHz from the PLL.
    //
    SysCtlClockSet(	SYSCTL_SYSDIV_2_5 |
    				SYSCTL_USE_PLL |
					SYSCTL_OSC_MAIN |
					SYSCTL_XTAL_16MHZ );

    //
    // Enable Interrupts
    //
    IntMasterEnable();
    IntEnable(INT_GPIOD);

    //
    // Initialize the UART as a console for text I/O.
    //
    ConfigureUART();


    dprintf("Systeminitialisierung ...\n", 0);

    //
	// Initialize DS3231 RTC
	//
    LEDInit();

    //
    // Initialize file manager
    //
    FM_Init();

    //
    // Initialize the VS1003 audio codec
    //
    VS10xxInit();

	//
	// Init task manager
	//
	TM_Init();

    //
    // Init timers
    //
    TimerInit();

    //
	// Initialize RLY8 relais board
	//
    ActInit();

    //
	// Initialize DS3231 RTC
	//
    DS3231Init();

    //
	// Initialize HCSR501 motion sensor
	//
    HCSR501Init();

    //
    // Init rotary encoder
    //
    //KY040Init();

	//
	// Initialize sMotionExitTime structure to -1
	//
	memset(&sMotionExitTime, -1, sizeof(sMotionExitTime));

	//
	// Mount file system, load configuration files and index available
	// files in root "/" directory
	//
    if(FM_SDCheck() == FR_OK)
    {
    	//
    	// Load configuration files
    	//
    	FM_ConfigLoad();

    	//
    	// Index files
    	//
    	ui16IndexedFiles = FM_Index();
    }

    //
    // Init Audio Manager
    //
    AM_Init(g_sSystem.ui8DefaultVolume, g_sSystem.ui8DefaultFadeTime, ui16IndexedFiles);

    //
	// Initialize display manager
	//
    DM_Init(g_sSystem.ui8DefaultDsplContrast, g_sSystem.ui8DefaultDsplBrightness);


    //
    // Clear display
    //
    NHD0216K3ZSendCommand(NHD0216K3Z_CLEAR_SCREEN, -1);

    //
    // Print hello message to user.
    //
    UARTprintf("\n\n########  ##    ##  ######   #######\n");
    UARTprintf("##     ##  ##  ##  ##    ## ##     ##\n");
    UARTprintf("##     ##   ####   ##       ##     ##\n");
    UARTprintf("########     ##    ##       ##     ##\n");
    UARTprintf("##           ##    ##       ##     ##\n");
    UARTprintf("##           ##    ##    ## ##     ##\n");
    UARTprintf("##           ##     ######   #######\n\n");

    UARTprintf("              Pyramidensteuerung v%s\n\n", FW_VERSION);

    UARTprintf("\nSchreibe \'?\' fuer Hilfe.\n\n");


    //
    // Reset task-running flag on startup
    //
    bTaskRunning = false;
    dprintf("Suche nach Aufgaben ... \n", 0);


	//
	// Print a prompt to the console.  Show the CWD.
	//
    // UARTprintf("\n%s> ", g_pcCwdBuf);

	//*********************************************************************************
    //
	// Enter an infinite loop.
	//
    //*********************************************************************************
	while(1)
	{
		//
		// Update display
		//
		DM_UpdateDisplay();

		//
		// Check for  mode
		// 0: Manuel
		// 1: Auto
		//
		if(g_sSystem.ui8Mode)
		{
			//
			// Simple do-while(0) approach to be able to exit and
			// return to beginning at any location
			//
			do {
				//
				// Check if a task is running
				//
				if(false == bTaskRunning)
				{
					//
					// Search task list for matching entries
					//
					i8TaskIdx = TM_TaskSearch(g_sDS3231Data.i32TimeInSeconds);

					//
					// In case a task was found got to "RUN" mode and use i8TaskIdx for further processing
					//
					if(i8TaskIdx >= 0)
					{
						dprintf("Aufgabe %i aktiviert, %02i:%02i - %02i:%02i\n", i8TaskIdx + 1, g_sTask[i8TaskIdx].sStartTime.i8Hour, g_sTask[i8TaskIdx].sStartTime.i8Minute, g_sTask[i8TaskIdx].sStopTime.i8Hour, g_sTask[i8TaskIdx].sStopTime.i8Minute);

						//
						// Go to "RUN" mode
						//
						bTaskRunning = true;

						//
						// Go to beginning
						//
						break;
					}
				}

				//
				// If task with index i8TaskIdx is running
				//
				else
				{
					//
					// Check if task stop time is reached
					//
					ui32CurrentTimeInSeconds = g_sDS3231Data.i32TimeInSeconds;

					if(ui32CurrentTimeInSeconds >= g_sTask[i8TaskIdx].sStopTime.i32TimeInSeconds)
					{
						dprintf("Aufgabe %i beendet.\n", i8TaskIdx + 1);

						//
						// Search in task list for matching entries
						//
						bSubseqTaskFound = false;

						dprintf("Suchen nach Aufgaben, die direkt nach Aufgabe %i folgen.\n", i8TaskIdx + 1);

						for(i8TaskIdx = 0; i8TaskIdx < N_TASK; i8TaskIdx++)
						{
							if(g_sTask[i8TaskIdx].sStartTime.i32TimeInSeconds >= 0)
							{
								if(g_sTask[i8TaskIdx].sStartTime.i32TimeInSeconds == ui32CurrentTimeInSeconds)
								{
									dprintf("Aufgabe %i gefunden.\n", i8TaskIdx + 1);
									bSubseqTaskFound = true;
									break;
								}
							}
						}

						//
						// Check subsequent task flag
						//
						if(false == bSubseqTaskFound)
						{

							dprintf("Keine direkt folgenden Aufgaben gefunden.\n", 0);

							//
							// Reset motion flags
							//
							g_bActiveMotionEvent = false;
							g_bExitMotionEvent = false;

							//
							// Disable motion sensor in case it is active
							//
							if(false != g_bHCSR501Enable)
							{
								//
								// Disable motion sensor
								//
								HCSR501Disable();
							}

							//
							// Disbale all actuator(s)
							//
							ActDisableAll();

							//
							// Fadeout and stop audio playback
							//
							AM_VolumeFade(g_sSystem.ui8DefaultFadeTime, AM_FADEOUT);

							//
							// Go to "SEARCH" mode
							//
							bTaskRunning = false;
							dprintf("Suche nach Aufgaben ... \n", 0);

							//
							// Go to beginning
							//
							break;
						}

						//
						// Go to beginning
						//
						break;
					} // if(ui32CurrentTimeInSeconds >= g_sTask[i8TaskIdx].sStopTime.i32TimeInSeconds)

					//
					// If task stop time is not reached
					//
					else
					{
						//
						// If motion sensing is enabled
						//
						if(false != g_sTask[i8TaskIdx].bMotion)
						{
							//
							// Check motion sensor state
							//
							if (false == g_bHCSR501Enable)
							{
								//
								// Enable motion sensor
								//
								HCSR501Enable();
							}

							//
							// MOTION ACTIVE EVENT
							//
							if(false != g_bActiveMotionEvent)
							{
								//
								// Rest motion exit time
								//
								memset(&sMotionExitTime, -1, sizeof(sMotionExitTime));
							}

							//
							// MOTION EXIT EVENT
							//
							else if(g_bExitMotionEvent) // if(g_bActiveMotionEvent)
							{
								//
								// Set motion exit time
								//
								if(sMotionExitTime.i8Hour == -1 && sMotionExitTime.i8Minute == -1 && sMotionExitTime.i8Second == -1)
								{
									//
									// Get current time
									//
									sMotionExitTime.i32TimeInSeconds = g_sDS3231Data.i32TimeInSeconds;
									ConvertSecondsInHhMmSs(sMotionExitTime.i32TimeInSeconds, &sMotionExitTime.i8Hour, &sMotionExitTime.i8Minute, &sMotionExitTime.i8Second);

									//
									// Calculate timeout time
									//
									sEventTimeout.i32TimeInSeconds = sMotionExitTime.i32TimeInSeconds + g_sTask[i8TaskIdx].sActiveTime.i32TimeInSeconds;
									ConvertSecondsInHhMmSs(sEventTimeout.i32TimeInSeconds, &sEventTimeout.i8Hour, &sEventTimeout.i8Minute, &sEventTimeout.i8Second);

									dprintf("Gueltiges Bewegungsereignis beende um %02i:%02i:%02i\n",sMotionExitTime.i8Hour, sMotionExitTime.i8Minute, sMotionExitTime.i8Second);
									dprintf("Event endet um %02i:%02i:%02i\n", sEventTimeout.i8Hour, sEventTimeout.i8Minute, sEventTimeout.i8Second);
								}

								//
								// Check if motion event time reached
								//
								if(sEventTimeout.i32TimeInSeconds <= g_sDS3231Data.i32TimeInSeconds)
								{
									//
									//Stop Event and reset all motion flags
									//
									g_bActiveMotionEvent = false;
									g_bExitMotionEvent = false;

									//
									// Disable all actuator(s)
									//
									ActDisableAll();

									//
									// Fadeout and Stop playback
									//
									AM_VolumeFade(g_sSystem.ui8DefaultFadeTime, AM_FADEOUT);

									dprintf("Bewegungsereignis beendet.\n", 0);

									//
									// Go to beginning
									//
									break;
								}
							} // if(false != g_bActiveMotionEvent)
							else
							{
								//
								// Go to beginning
								//
								break;
							}
						} // if(g_sTask[ui8Idx].bMotion)
						else
						{
							//
							// Check motion sensor state
							//
							if(g_bHCSR501Enable)
							{
								//
								// Disable motion sensor
								//
								HCSR501Disable();
							}
						}

						//
						// If Audio playback is enabled
						//
						if(false != g_sTask[i8TaskIdx].bAudio)
						{
							//
							// Start audio playback
							//
							AM_Play();
						}
						else
						{
							//
							// Fade out and stop audio playback
							//
							AM_VolumeFade(g_sSystem.ui8DefaultFadeTime, AM_FADEOUT);
						}


						//
						// If actor usage is enabled
						//
						if(false != g_sTask[i8TaskIdx].bActuator)
						{
							//
							// Enable Actuator(s)
							//
							for(ui8Cnt = 0; ui8Cnt < ACT_N_CH; ui8Cnt++)
							{
								if(g_sTask[i8TaskIdx].bActuatorCH[ui8Cnt] == true)
								{
									ActEnbale(ui8Cnt + 1);
								}
								else
								{
									ActDisable(ui8Cnt + 1);
								}
							}
						}
						else
						{
							//
							// Disable all actor
							//
							// ActDisableAll();
						}
					} // if(ui32CurrentTimeInSeconds >= g_sTask[i8TaskIdx].sStopTime.i32TimeInSeconds)
				} // if(false == bTaskRunning)
			}
			while(0);
		}


		//**********************************************************************
		//
		// Commandline processing
		//
		//**********************************************************************

        //
        // Peek to see if a full command is ready for processing
        //
		if(UARTPeek('\r') != -1)
		{
			//
			// Get a line of text from the user.
			//
			UARTgets(g_pcCmdBuf, sizeof(g_pcCmdBuf));

			//
			// Pass the line from the user to the command processor.  It will be
			// parsed and valid commands executed.
			//
			nStatus = CmdLineProcess(g_pcCmdBuf);

			//
			// Handle the case of bad command.
			//

			if(nStatus == CMDLINE_BAD_CMD)
			{
				dprintf("Falsches Kommando!\n", 0);
			}

			//
			// Handle the case of too many arguments.
			//
			else if(nStatus == CMDLINE_TOO_MANY_ARGS)
			{
				eprintf("Too many arguments for command processor!\n", 0);
			}

			//
			// Otherwise the command was executed.  Print the error code if one was
			// returned.
			//
			else if(nStatus != 0)
			{
				// UARTprintf("Command returned error code %s\n",
				//		StringFromFResult((FRESULT)nStatus));
			}

			//
			// Print a prompt to the console.  Show the CWD.
			//
			UARTprintf("\n%s> ", g_pcCwdBuf);
		}
	}
}
