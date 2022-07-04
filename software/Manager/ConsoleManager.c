/*
 * ColsoleManager.c
 *
 *  Created on: 08.12.2015
 *      Author: Local
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
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "grlib/grlib.h"
#include "utils/cmdline.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "../Driver/fatfs/ff.h"
#include "../Driver/fatfs/diskio.h"
#include "../Driver/VS10xx.h"
#include "../Driver/DS3231.h"
#include "FileManager.h"
#include "ConsoleManager.h"
#include "../Driver/ACT.h"
#include "AudioManager.h"
#include "../Driver/NHD0216K3Z.h"
#include "../Driver/Timer.h"
#include "TaskManager.h"
#include "../Msg.h"
#include "../System.h"


//*****************************************************************************
//
// Printf functions used in this file
//
//*****************************************************************************
#define FILE_ID				("COMG")
#define eprintf(fms, ...)	do{	UARTprintf("\033[31;1m"); \
								UARTprintf("[ %s ] ERROR " fms, FILE_ID, __VA_ARGS__); \
								UARTprintf("\033[0m");} while(0)

#define dprintf(fms, ...)	UARTprintf("[ %s ] " fms, FILE_ID, __VA_ARGS__); \




//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>SD card using FAT file system (sd_card)</h1>
//!
//! This example application demonstrates reading a file system from an SD
//! card.  It makes use of FatFs, a FAT file system driver.  It provides a
//! simple command console via a serial port for issuing commands to view and
//! navigate the file system on the SD card.
//!
//! The first UART, which is connected to the USB debug virtual serial port on
//! the evaluation board, is configured for 115,200 bits per second, and 8-N-1
//! mode.  When the program is started a message will be printed to the
//! terminal.  Type ``help'' for command help.
//!
//! For additional details about FatFs, see the following site:
//! http://elm-chan.org/fsw/ff/00index_e.html
//
//*****************************************************************************


//*****************************************************************************
//
// This function implements the "cd" command.  It takes an argument that
// specifies the directory to make the current working directory.  Path
// separators must use a forward slash "/".  The argument to cd can be one of
// the following:
//
// * root ("/")
// * a fully specified path ("/my/path/to/mydir")
// * a single directory name that is in the current directory ("mydir")
// * parent directory ("..")
//
// It does not understand relative paths, so dont try something like this:
// ("../my/new/path")
//
// Once the new directory is specified, it attempts to open the directory to
// make sure it exists.  If the new path is opened successfully, then the
// current working directory (cwd) is changed to the new path.
//
//*****************************************************************************
int
Cmd_cd(int argc, char *argv[])
{

	FM_cd(argc, argv);

	return 0;
}

int
Cmd_index(int argc, char *argv[])
{

	FM_Index();

	return 0;
}



//*****************************************************************************
//
// This function implements the "pwd" command.  It simply prints the current
// working directory.
//
//*****************************************************************************
int
Cmd_pwd(int argc, char *argv[])
{
    //
    // Print the CWD to the console.
    //
    UARTprintf("%s\n", g_pcCwdBuf);

    //
    // Return success.
    //
    return(0);
}

//*****************************************************************************
//
// This function implements the "cat" command.  It reads the contents of a file
// and prints it to the console.  This should only be used on text files.  If
// it is used on a binary file, then a bunch of garbage is likely to printed on
// the console.
//
//*****************************************************************************
int
Cmd_cat(int argc, char *argv[])
{

	FM_cat(argc, argv);
	return 0;
}


//*****************************************************************************
//
// This function implements the "cat" command.  It reads the contents of a file
// and prints it to the console.  This should only be used on text files.  If
// it is used on a binary file, then a bunch of garbage is likely to printed on
// the console.
//
//*****************************************************************************
int
Cmd_deleteFile(int argc, char *argv[])
{
	FM_FileDelete(argv[1]);

	return 0;
}




//*****************************************************************************
//
// This function implements the "cat" command.  It reads the contents of a file
// and prints it to the console.  This should only be used on text files.  If
// it is used on a binary file, then a bunch of garbage is likely to printed on
// the console.
//
//*****************************************************************************
int
Cmd_ls(int argc, char *argv[])
{
	FM_ls();

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
int
Cmd_play(int argc, char *argv[])
{
	VS10xxFilePathSet(argv[1]);
	VS10xxPlay();

    return(0);
}

int
Cmd_fadein(int argc, char *argv[])
{
	uint8_t ui8Val = (uint8_t) ustrtoul(argv[1], 0, 0);

	AM_VolumeFade(ui8Val, AM_FADEIN);

    return(0);
}


int
Cmd_fadeout(int argc, char *argv[])
{
	uint8_t ui8Val = (uint8_t) ustrtoul(argv[1], 0, 0);

	AM_VolumeFade(ui8Val, AM_FADEOUT);

    return(0);
}




int
Cmd_stop(int argc, char *argv[])
{
	VS10xxStop();

    return(0);
}


int
Cmd_pause(int argc, char *argv[])
{
	VS10xxPause();

    return(0);
}



int
Cmd_AudioSetVolume(int argc, char *argv[])
{
	char* val;

	if (getarg(&argc, &argv, "-s", &val ) )
	{
		AM_VolumeSet(ustrtoul(val, 0, 0));
	}

	if (getarg(&argc, &argv, "-m", &val ) )
	{
		VS10xxVolumeMute();
	}

	return(0);
}





uint8_t g_pui8DS3231RegVal[DS3231_N_REG] = {0};


//*****************************************************************************
//
// RTC commands
//
//*****************************************************************************

//*****************************************************************************
//
// Table of valid command strings, callback functions and help messages.  This
// is used by the cmdline module.
//
//*****************************************************************************
tCmdLineParamEntry g_psRtcCmdLineParamTable[] =
{
        { "-?",         "Liste aller verfuegbaren Parameter" },
        { "-a",        "Datum, Zeit und Temperatur anzeigen" },
        { "-wt",        "Setzt den Wochentag: Mo, Di, Mi, Do, Fr, Sa, So", "-wt <Wochentag>" },
        { "-dd",        "Setzt den Tag im Monat: 1 - 31", "-dd <Tag>" ,1, 31 },
        { "-mm",        "Setzt den Monat: 1 - 12", "-mm <Monat>" ,1, 12 },
        { "-yy",        "Setzt das Jahr: 0 - 99", "-yy <Jahr>", 1, 99 },
        { "-h",         "Setzt die Stunde: 0 - 23", "-h <Stunde>", 0, 23 },
        { "-m",         "Setzt die Minute: 0 - 59", "-m <Minute>", 0, 59 },
        { "-s",         "Setzt die Sekunde: 0 - 59", "-s <Sekunde>", 0, 59 },
        { "-al",        "Alle Register anzeigen" },
        { 0, 0 }
};

//*****************************************************************************
//
// Command: intensity
//
// Takes a single argument that is between zero and one hundred. The argument
// must be an integer.  This is interpreted as the percentage of maximum
// brightness with which to display the current color.
//
//*****************************************************************************
int
CMD_rtc(int argc, char **argv)
{
    int32_t i32Index;
    uint8_t ui8Day;
    //uint8_t pui8RtcData[DS3231_N_DATA];
    float fTemperature;
    char pcBuffer[6];
    char* val;
    char Dec2BinStr[9];
    bool bDayOk;
    uint8_t ui8CmdLineParamVal;
    char* cMonthName[] = {"Januar", "Februar", "Maerz", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"};
    // char* cDayName[] = {"Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"};
    //DS3231* psDate;

    //
    // Print help on console
    //
    if (getarg(&argc, &argv, "-?", &val ) )
    {
       //
       // Start at the beginning of the command table
       //
       i32Index = 0;
       UARTprintf("\nECHTZEITUHR\n");
       UARTprintf("zeit -<Parameter> <Wert>\n");

       //
       // Leave a blank line after the help strings.
       //
       UARTprintf("\n");

       //
       // Display strings until we run out of them.
       //
       while(g_psRtcCmdLineParamTable[i32Index].pcParam)
       {
         UARTprintf("%5s %s\n      %s\n", g_psRtcCmdLineParamTable[i32Index].pcParam,
                                g_psRtcCmdLineParamTable[i32Index].pcHelp,
                                g_psRtcCmdLineParamTable[i32Index].pcUsage);
         i32Index++;
       }

       //
       // Leave a blank line after the help strings.
       //
       UARTprintf("\n");
    }

	//
	// Set day of week (Wochentag)
	//
	if (getarg(&argc, &argv, g_psRtcCmdLineParamTable[2].pcParam, &val ) )
	{
		if(!strcmp(val, "Mo")){ui8Day = 1; UARTprintf("Setzte Wochentag zu Montag..."); bDayOk = true;}
		else if(!strcmp(val, "Di")){ui8Day = 2; UARTprintf("Setzte Wochentag zu Dienstag..."); bDayOk = true;}
		else if(!strcmp(val, "Mi")){ui8Day = 3; UARTprintf("Setzte Wochentag zu Mittwoch..."); bDayOk = true;}
		else if(!strcmp(val, "Do")){ui8Day = 4; UARTprintf("Setzte Wochentag zu Donnerstag..."); bDayOk = true;}
		else if(!strcmp(val, "Fr")){ui8Day = 5; UARTprintf("Setzte Wochentag zu Freitag..."); bDayOk = true;}
		else if(!strcmp(val, "Sa")){ui8Day = 6; UARTprintf("Setzte Wochentag zu Samstag..."); bDayOk = true;}
		else if(!strcmp(val, "So")){ui8Day = 7; UARTprintf("Setzte Wochentag zu Sonntag..."); bDayOk = true;}
		else{UARTprintf("\nFehler: Tag nicht vorhanden!\n"); bDayOk = false;}

		//
		// Check if day set is correct
		//
		if(bDayOk)
		{
			if(DS3231DaySet(ui8Day))
			{
				UARTprintf("OK\n");
			}
			else
			{
				UARTprintf("FEHLER\n");
				UARTprintf("\nFehler: Keine Verbindung zur Echtzeituhr!\n");
			}
		}
	}

	//
	// Set date
	//
	if (getarg(&argc, &argv, g_psRtcCmdLineParamTable[3].pcParam, &val ) )
	{
		ui8CmdLineParamVal = (uint8_t) ustrtoul(val, 0, 0);

		if(ui8CmdLineParamVal >= g_psRtcCmdLineParamTable[3].ui8ValMin && ui8CmdLineParamVal <= g_psRtcCmdLineParamTable[3].ui8ValMax)
		{
			UARTprintf("Setzte Tag zu %02d...", ui8CmdLineParamVal);

			if(DS3231DateSet(ui8CmdLineParamVal))
			{
				UARTprintf("OK\n");
			}
			else
			{
				UARTprintf("\nFehler: Keine Verbindung zur Echtzeituhr!\n");
			}
		}
		else
		{
			UARTprintf("\nFehler: Wert auserhalb des erlaubten Bereiches: %d - %d\n", g_psRtcCmdLineParamTable[3].ui8ValMin, g_psRtcCmdLineParamTable[3].ui8ValMax);
		}
	}

    //
    // Set month
    //
    if (getarg(&argc, &argv, g_psRtcCmdLineParamTable[4].pcParam, &val ) )
    {
        ui8CmdLineParamVal = (uint8_t) ustrtoul(val, 0, 0);

        if(ui8CmdLineParamVal >= g_psRtcCmdLineParamTable[4].ui8ValMin && ui8CmdLineParamVal <= g_psRtcCmdLineParamTable[4].ui8ValMax)
        {
            UARTprintf("Setzte Monat zu %s...",  cMonthName[ui8CmdLineParamVal-1]);

            if(DS3231MonthSet(ui8CmdLineParamVal))
            {
                UARTprintf("OK\n");
            }
            else
            {
                UARTprintf("\nFehler: Keine Verbindung zur Echtzeituhr!\n");
            }
        }
        else
        {
            UARTprintf("\nFehler: Wert auserhalb des erlaubten Bereiches: %d - %d\n", g_psRtcCmdLineParamTable[4].ui8ValMin, g_psRtcCmdLineParamTable[4].ui8ValMax);
        }
    }

    //
    // Set year
    //
    if (getarg(&argc, &argv, g_psRtcCmdLineParamTable[5].pcParam, &val ) )
    {
        ui8CmdLineParamVal = (uint8_t) ustrtoul(val, 0, 0);

        if(ui8CmdLineParamVal >= g_psRtcCmdLineParamTable[5].ui8ValMin && ui8CmdLineParamVal <= g_psRtcCmdLineParamTable[5].ui8ValMax)
        {
            UARTprintf("Setzte Jahr zu %d...",  ui8CmdLineParamVal);

            if(DS3231YearSet(ui8CmdLineParamVal))
            {
                UARTprintf("OK\n");
            }
            else
            {
                UARTprintf("\nFehler: Keine Verbindung zur Echtzeituhr!\n");
            }
        }
        else
        {
            UARTprintf("\nFehler: Wert auserhalb des erlaubten Bereiches: %d - %d\n", g_psRtcCmdLineParamTable[5].ui8ValMin, g_psRtcCmdLineParamTable[5].ui8ValMax);
        }
    }

    //
    // Set Hour
    //
    if (getarg(&argc, &argv, g_psRtcCmdLineParamTable[6].pcParam, &val ) )
    {
        ui8CmdLineParamVal = (uint8_t) ustrtoul(val, 0, 0);

        if(ui8CmdLineParamVal >= g_psRtcCmdLineParamTable[6].ui8ValMin && ui8CmdLineParamVal <= g_psRtcCmdLineParamTable[6].ui8ValMax)
        {
            UARTprintf("Setzte Stunde zu %d...",  ui8CmdLineParamVal);

            if(DS3231HourSet(ui8CmdLineParamVal))
            {
                UARTprintf("OK\n");
            }
            else
            {
                UARTprintf("\nFehler: Keine Verbindung zur Echtzeituhr!\n");
            }
        }
        else
        {
            UARTprintf("\nFehler: Wert auserhalb des erlaubten Bereiches: %d - %d\n", g_psRtcCmdLineParamTable[6].ui8ValMin, g_psRtcCmdLineParamTable[6].ui8ValMax);
        }
    }

        //
        // Set Minute
        //
        if (getarg(&argc, &argv, g_psRtcCmdLineParamTable[7].pcParam, &val ) )
        {
            ui8CmdLineParamVal = (uint8_t) ustrtoul(val, 0, 0);

            if(ui8CmdLineParamVal >= g_psRtcCmdLineParamTable[7].ui8ValMin && ui8CmdLineParamVal <= g_psRtcCmdLineParamTable[7].ui8ValMax)
            {
                UARTprintf("Setzte Minute zu %d...",  ui8CmdLineParamVal);

                if(DS3231MinuteSet(ui8CmdLineParamVal))
                {
                    UARTprintf("OK\n");
                }
                else
                {
                    UARTprintf("\nFehler: Keine Verbindung zur Echtzeituhr!\n");
                }
            }
            else
            {
                UARTprintf("\nFehler: Wert auserhalb des erlaubten Bereiches: %d - %d\n", g_psRtcCmdLineParamTable[6].ui8ValMin, g_psRtcCmdLineParamTable[6].ui8ValMax);
            }
        }

        //
        // Set Second
        //
        if (getarg(&argc, &argv, g_psRtcCmdLineParamTable[8].pcParam, &val ) )
        {
            ui8CmdLineParamVal = (uint8_t) ustrtoul(val, 0, 0);

            if(ui8CmdLineParamVal >= g_psRtcCmdLineParamTable[8].ui8ValMin && ui8CmdLineParamVal <= g_psRtcCmdLineParamTable[8].ui8ValMax)
            {
                UARTprintf("Setzte Sekunde zu %d...",  ui8CmdLineParamVal);

                if(DS3231SecondSet(ui8CmdLineParamVal))
                {
                    UARTprintf("OK\n");
                }
                else
                {
                    UARTprintf("\nFehler: Keine Verbindung zur Echtzeituhr!\n");
                }
            }
            else
            {
                UARTprintf("\nFehler: Wert auserhalb des erlaubten Bereiches: %d - %d\n", g_psRtcCmdLineParamTable[8].ui8ValMin, g_psRtcCmdLineParamTable[8].ui8ValMax);
            }
        }

        //
        // Get and display all registers
        //
        if (getarg(&argc, &argv, g_psRtcCmdLineParamTable[9].pcParam, &val))
        {
            UARTprintf("Lese alle register...");

            if(DS3231AllRegisterGet(g_pui8DS3231RegVal, DS3231_N_REG))
            {
                UARTprintf("OK\n");

                for (i32Index = 0; i32Index < DS3231_N_REG; i32Index++)
                {
                    Dec2Bin(g_pui8DS3231RegVal[i32Index], 8, Dec2BinStr);
                    UARTprintf("Register 0x%2x: 0x%3x, %s\n", i32Index, g_pui8DS3231RegVal[i32Index], Dec2BinStr);
                }
            }
            else
            {
                UARTprintf("FEHLER\n.");

                UARTprintf("\nFehler: Keine Verbindung zur Echtzeituhr!\n");
            }
        }

    //
    // Display date, time and temperature
    //
    if (getarg(&argc, &argv, g_psRtcCmdLineParamTable[1].pcParam, &val ) )
    {
        //
        // Get time and date
        //
		DS3231TemperatureGet(&fTemperature);

		snprintf(pcBuffer, 6, "%f", fTemperature);
		UARTprintf("--------------------------------\n");
		UARTprintf("%s %02d.%02d.%02d %02d:%02d:%02d  %s Grad\n",
					g_sDS3231Data.ui8DayOfWeek,
					g_sDS3231Data.ui8Day,
					g_sDS3231Data.ui8Month,
					g_sDS3231Data.ui8Year,
					g_sDS3231Data.ui8Hour,
					g_sDS3231Data.ui8Minute,
					g_sDS3231Data.ui8Second,
					pcBuffer);

		UARTprintf("--------------------------------\n");

    }
	return(0);
}






//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int
Cmd_rly(int argc, char *argv[])
{
	uint8_t ui8CmdLineParamVal;
	char* val;

	if (getarg(&argc, &argv, "-an", &val ) )
	{
		ui8CmdLineParamVal = (uint8_t) ustrtoul(val, 0, 0);
		ActEnbale(ui8CmdLineParamVal);

	}
	else if (getarg(&argc, &argv, "-aus", &val ) )
	{
		ui8CmdLineParamVal = (uint8_t) ustrtoul(val, 0, 0);
		ActDisable(ui8CmdLineParamVal);
	}
	else if (getarg(&argc, &argv, "-allean", &val ) )
	{
		ActEnbaleAll();
	}
	else if (getarg(&argc, &argv, "-alleaus", &val ) )
	{
		ActDisableAll();
	}
	else
	{
		UARTprintf("Befehl nicht vorhanden.\n");
	}

    //
    // Return success.
    //
    return(0);
}




//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int
Cmd_disp(int argc, char *argv[])
{
	uint8_t ui8CmdVal;
	char* cVal;

	if (getarg(&argc, &argv, "-c", &cVal ) )
	{
		ui8CmdVal = (uint8_t) ustrtoul(cVal, 0, 0);
		NHD0216K3ZSetContrast(ui8CmdVal);

	}
	else if (getarg(&argc, &argv, "-b", &cVal ) )
	{
		ui8CmdVal = (uint8_t) ustrtoul(cVal, 0, 0);
		NHD0216K3ZSetBrightness(ui8CmdVal);
	}
	else if (getarg(&argc, &argv, "-an", &cVal ) )
	{
		NHD0216K3ZOn();
	}
	else if (getarg(&argc, &argv, "-aus", &cVal ) )
	{
		NHD0216K3ZOff();
	}
	else
	{
		UARTprintf("Befehl nicht vorhanden.\n");
	}

    //
    // Return success.
    //
    return(0);
}



//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int
Cmd_cls(int argc, char *argv[])
{
	//
	// Clear console window
	//
	UARTprintf("\033[2J\033[1;1H");

    //
    // Return success.
    //
    return(0);
}



//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int
Cmd_TaskConfigSave(int argc, char *argv[])
{
	FM_ConfigSave(F_TASK);

    //
    // Return success.
    //
    return(0);
}


//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int
Cmd_SysConfigSave(int argc, char *argv[])
{
	FM_ConfigSave(F_SYS);

    //
    // Return success.
    //
    return(0);
}


//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int
Cmd_configload(int argc, char *argv[])
{
	FM_ConfigLoad();

    //
    // Return success.
    //
    return(0);
}


//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int
Cmd_TaskAdd(int argc, char *argv[])
{
	char* 	cVal;

	TIME	sStartTime;
	TIME	sStopTime;
	TIME	sActiveTime;
	bool	bMotion;
	bool	bAudio ;
	bool	bActuator;
	uint8_t	ui8ActuatorProg = 0;


	memset(&sStartTime, 0, sizeof(sStartTime));
	memset(&sStopTime, 0, sizeof(sStopTime));
	memset(&sActiveTime, 0, sizeof(sActiveTime));
	bMotion = false;
	bAudio = false;
	bActuator = false;
	ui8ActuatorProg = 0;


	if (getarg(&argc, &argv, "-sah", &cVal ) )
	{
		sStartTime.i8Hour = (uint8_t) ustrtoul(cVal, 0, 0);
	}

	if (getarg(&argc, &argv, "-sam", &cVal ) )
	{
		sStartTime.i8Minute = (uint8_t) ustrtoul(cVal, 0, 0);
	}

	if (getarg(&argc, &argv, "-soh", &cVal ) )
	{
		sStopTime.i8Hour = (uint8_t) ustrtoul(cVal, 0, 0);
	}

	if (getarg(&argc, &argv, "-som", &cVal ) )
	{
		sStopTime.i8Minute = (uint8_t) ustrtoul(cVal, 0, 0);
	}

	if (getarg(&argc, &argv, "-b", &cVal ) )
	{
		bMotion = (uint8_t) ustrtoul(cVal, 0, 0);
	}

	if (getarg(&argc, &argv, "-au", &cVal ) )
	{
		bAudio = (uint8_t) ustrtoul(cVal, 0, 0);
	}

	if (getarg(&argc, &argv, "-ak", &cVal ) )
	{
		ui8ActuatorProg = (uint8_t) ustrtoul(cVal, 0, 0);
	}



	/*
	else
	{
		eprintf("Parameter nicht vorhanden.\n", 0);
		return 0;
	}
*/

	TM_TaskAdd(	sStartTime,
				sStopTime,
				sActiveTime,
				bMotion,
				bAudio,
				bActuator,
				ui8ActuatorProg);

	TM_TaskPrint();

    //
    // Return success.
    //
    return(0);
}



//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int
Cmd_TaskSet(int argc, char *argv[])
{
	char* 	cVal;
	uint8_t ui8TaskId;


	if (getarg(&argc, &argv, "-a", &cVal ) )
	{
		ui8TaskId = (uint8_t) ustrtoul(cVal, 0, 0);

		if (getarg(&argc, &argv, "-sah", &cVal ) )
		{
			TM_TaskSet(ui8TaskId, STARTTIME_H, ustrtoul(cVal, 0, 0));
		}

		if (getarg(&argc, &argv, "-sam", &cVal ) )
		{
			TM_TaskSet(ui8TaskId, STARTTIME_M, ustrtoul(cVal, 0, 0));
		}

		if (getarg(&argc, &argv, "-soh", &cVal ) )
		{
			TM_TaskSet(ui8TaskId, STOPTIME_H, ustrtoul(cVal, 0, 0));
		}

		if (getarg(&argc, &argv, "-som", &cVal ) )
		{
			TM_TaskSet(ui8TaskId, STOPTIME_M, ustrtoul(cVal, 0, 0));
		}

		if (getarg(&argc, &argv, "-akm", &cVal ) )
		{
			TM_TaskSet(ui8TaskId, ACTIVETIME_M, ustrtoul(cVal, 0, 0));
		}

		if (getarg(&argc, &argv, "-aks", &cVal ) )
		{
			TM_TaskSet(ui8TaskId, ACTIVETIME_S, ustrtoul(cVal, 0, 0));
		}

		if (getarg(&argc, &argv, "-b", &cVal ) )
		{
			TM_TaskSet(ui8TaskId, MOTION, ustrtoul(cVal, 0, 0));
		}

		if (getarg(&argc, &argv, "-au", &cVal ) )
		{
			TM_TaskSet(ui8TaskId, AUDIO, ustrtoul(cVal, 0, 0));
		}

		if (getarg(&argc, &argv, "-akt", &cVal ) )
		{
			TM_TaskSet(ui8TaskId, ACTUATOR, ustrtoul(cVal, 0, 0));
		}

		if (getarg(&argc, &argv, "-ak", &cVal ) )
		{
			TM_TaskSet(ui8TaskId, ACTUATOR_CH, ustrtoul(cVal, 0, 0));
		}
	}
	else
	{
		eprintf("Bitte Aufgaben Id angeben!\n", 0);
		return 0;
	}

	TM_TaskPrint();

    //
    // Return success.
    //
    return(0);
}




//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int
Cmd_TaskRemove(int argc, char *argv[])
{
	TM_TaskRemove((uint8_t) ustrtoul(argv[1], 0, 0));
	TM_TaskPrint();

    //
    // Return success.
    //
    return(0);
}

//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int
Cmd_TaskPrint(int argc, char *argv[])
{
	TM_TaskPrint();

    //
    // Return success.
    //
    return(0);
}



Cmd_SysParamPrint(int argc, char *argv[])
{
	SYS_ParamsPrint();
}



//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int
Cmd_SysParamSet(int argc, char *argv[])
{
	char* 	cVal;
	char*	cVal1;

	if (getarg(&argc, &argv, "-m", &cVal ) )
	{
		SYS_ModusSet(ustrtoul(cVal, 0, 0));
	}

	if (getarg(&argc, &argv, "-v", &cVal ) )
	{
		SYS_DefaultVolumeSet(ustrtoul(cVal, 0, 0));
	}

	if (getarg(&argc, &argv, "-ebz", &cVal))
	{
		SYS_DefaulFadeTime(ustrtoul(cVal, 0, 0));
	}

	if (getarg(&argc, &argv, "-a", &cVal ) )
	{
		SYS_AutosaveSet(ustrtoul(cVal, 0, 0));
	}

	if (getarg(&argc, &argv, "-dc", &cVal ) )
	{
		SYS_DefaultDispContrastSet(ustrtoul(cVal, 0, 0));
	}

	if (getarg(&argc, &argv, "-db", &cVal ) )
	{
		SYS_DefaultDispBrightnessSet(ustrtoul(cVal, 0, 0));
	}

	if (getarg(&argc, &argv, "-atm", &cVal ) && getarg(&argc, &argv, "-ats", &cVal1 ))
	{
		SYS_DefaultActiveTineSet(ustrtoul(cVal, 0, 0), ustrtoul(cVal1, 0, 0));
	}

	if (getarg(&argc, &argv, "-bvz", &cVal))
	{
		SYS_DefaultMotionDelay(ustrtoul(cVal, 0, 0));
	}

	SYS_ParamsPrint();

    //
    // Return success.
    //
    return(0);
}




//*****************************************************************************
//
// RTC commands
//
//*****************************************************************************

//*****************************************************************************
//
// Command: intensity
//
// Takes a single argument that is between zero and one hundred. The argument
// must be an integer.  This is interpreted as the percentage of maximum
// brightness with which to display the current color.
//
//*****************************************************************************
int getarg(int* argc, char*** argv, char* arg, char** val)
{
    int i = 0;

    for ( i = 0 ; i < *argc; ++i)
    {
        if ( ! strcmp ( (*argv)[i], arg ) )
        { /* -- match -- */

            /* -- if last arg or next arg is a '-' assume no value.
             *    Remove arg and return                            -- */
            if ( i == (*argc) - 1 || (*argv)[i+1][0] == '-')
            {
                *val = 0; /* -- let caller know there was no value -- */
                (*argc) -- ;
                while ( i < *argc )
                {(*argv)[i] = (*argv)[i+1]; ++i;}
                return (1);
            }

            /* -- arg has value -- */
            *val = (*argv)[i+1];
            (*argc) -= 2;
            while ( i < *argc )
            {(*argv)[i] = (*argv)[i+2]; ++i;}
            return (1);
        }
    }

    return (0); /* -- no matches found -- */
} /* -- getarg -- */







void Dec2Bin(int value, int bitsCount, char* output)
{
    int i;
    output[bitsCount] = '\0';
    for (i = bitsCount - 1; i >= 0; --i, value >>= 1)
    {
        output[i] = (value & 1) + '0';
    }
}






//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int
Cmd_help(int argc, char *argv[])
{
    tCmdLineEntry *psEntry;

    //
    // Print some header text.
    //
    UARTprintf("\n+---------------------------------------------------------+\n");
      UARTprintf("|                         BEFEHLE                         |\n");
      UARTprintf("+---------------------------------------------------------+\n\n");

    //
    // Point at the beginning of the command table.
    //
    psEntry = &g_psCmdTable[0];

    //
    // Enter a loop to read each entry from the command table.  The end of the
    // table has been reached when the command name is NULL.
    //
    while(psEntry->pcCmd)
    {

    	SysCtlDelay(80000);

        //
        // Print the command name and the brief description.
        //
        UARTprintf("%8s: %s\n", psEntry->pcCmd, psEntry->pcHelp);

        //
        // Advance to the next entry in the table.
        //
        psEntry++;
    }

    //
    // Return success.
    //
    return(0);
}

//*****************************************************************************
//
// This is the table that holds the command names, implementing functions, and
// brief description.
//
//*****************************************************************************
tCmdLineEntry g_psCmdTable[] =
{
    { "?",      	Cmd_help,   		"Alle Kommandos anzeigen" },
	{ "cls",		Cmd_cls,			"Konsole saeubern" },
	{ "ls",     	Cmd_ls,     		"Liste gesamtne Inhalt der SD Karte" },
	{ "cd",     	Cmd_cd,     		"Wechsle in Ornder: cd <Ordner>" },
    { "pwd",    	Cmd_pwd,    		"Zeige aktuellen Arbeotspfad an" },
    { "cat",    	Cmd_cat,    		"Zeige Dateiinhalt an: cat <Datei>" },
	{ "rm",			Cmd_deleteFile,		"Loesche Datei oder Ordner: rm <Datei/Ordner>" },
	{ "index",		Cmd_index,			"Indiziert alle Dateien auf der SD-Karte" },
    { "play",   	Cmd_play,   		"Spiele eine Audiodatei ab: play <Audiodatei>, Formate: .mp3, .wav" },
	{ "pause",  	Cmd_pause,   		"Pausiere die aktuelle Wiedergabe" },
	{ "stop",  		Cmd_stop,   		"Stoppe die aktuelle Wiedergabe" },
	{ "lst", 		Cmd_AudioSetVolume,	"Lautstaerke: vol -s <Wert:0-100>, Lautlos (mute): vol -m" },
	{ "einbl",	 	Cmd_fadein,  		"Audio einblenden: einbl <ZeitInSekunden>" },
	{ "ausbl", 		Cmd_fadeout,  		"Audio ausblenden: ausbl <ZeitInSekunden>" },
	{ "z",			CMD_rtc,			"Echtzeit Uhr: z -? fuer Hilfe" },
	{ "rly",		Cmd_rly,			"Relais An/Aus: rly -an <Kanal:1-8>, rly -aus <Kanal:1-8>, rly -allean, rly -alleaus" },
	{ "disp",		Cmd_disp,			"Display Einstellungen,  Kontrast: disp -c <Wert:1:50>, Helligkeit: disp -b <Wert:1:8>" },
	{ "spa",		Cmd_TaskConfigSave,	"Speichere der Aufgabenkonfiguration " },
	{ "sps",		Cmd_SysConfigSave,	"Speichere der Systemkonfiguration " },
	{ "lad",		Cmd_configload,		"Lade alle Konfigurationen " },
	{ "ah",			Cmd_TaskAdd,		"Neue Aufgabe hinzufügen." },
	{ "ae",			Cmd_TaskRemove,		"Aufgabe entfernen." },
	{ "ab",			Cmd_TaskSet,		"Aufgabe bearbeiten." },
	{ "aa",			Cmd_TaskPrint,		"Alle Aufgaben auflisten." },
	{ "sa",			Cmd_SysParamPrint,	"Systemparameter anzeigen." },
	{ "sb",			Cmd_SysParamSet,	"Systemparameter bearbeiten." },
    { 0, 0, 0 }
};



