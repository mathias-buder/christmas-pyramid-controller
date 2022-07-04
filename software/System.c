/*
 * System.c
 *
 *  Created on: 13.11.2016
 *      Author: mbuder
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "System.h"



//*****************************************************************************
//
// Default system settings
//
//*****************************************************************************
SYSTEM g_sSystem = {
	1,			// Mode: Manuel
	70,			// Default volume: 70%
	10,			// Default fade-in/out time (s)
	false,		// Disable auto-save-mode
	30,			// Default display contrast (0 - 50)
	8,			// Default display brightness (0 -8)
	{0, 5, 0},	// Default active time 00:05:00
	5			// Default motion delay 10 seconds
};


void
SYS_ParamsPrint(void)
{
	UARTprintf("+------------------------------------------+\n");
	UARTprintf("|             Systemparameter              |\n");
	UARTprintf("+--------------------+---------------------+\n");
	UARTprintf("| Modus              | %s\n", g_sSystem.ui8Mode ? "Auto" : "Manuel");
	UARTprintf("| Lautstaerke        | %i%%\n", g_sSystem.ui8DefaultVolume);
	UARTprintf("| Ein/Ausblendzeit   | %i (s)\n", g_sSystem.ui8DefaultFadeTime);
	// UARTprintf("| Autosave           | %s\n", g_sSystem.bAutoSave ? "Aktiviert" : "Deaktiviert");
	UARTprintf("| Displaykontrast    | %i\n", g_sSystem.ui8DefaultDsplContrast);
	UARTprintf("| Displayheligkeit   | %i\n", g_sSystem.ui8DefaultDsplBrightness);
	UARTprintf("| Standart Aktivzeit | %02i:%02i (mm:ss)\n", g_sSystem.sDefaultActiveTime.i8Minute, g_sSystem.sDefaultActiveTime.i8Second);
	UARTprintf("| Verzoegerungszeit  | %02i (s)\n", g_sSystem.ui8DefaultMotionDelay);
	UARTprintf("+--------------------+---------------------+\n");
}

void
SYS_ModusSet(uint8_t ui8Param)
{
	 g_sSystem.ui8Mode = ui8Param;
}

void
SYS_DefaultVolumeSet(uint8_t ui8Param)
{
	g_sSystem.ui8DefaultVolume = ui8Param;
}

void
SYS_DefaulFadeTime(uint8_t ui8Param)
{
	g_sSystem.ui8DefaultFadeTime = ui8Param;
}

void
SYS_AutosaveSet(uint8_t ui8Param)
{
	g_sSystem.bAutoSave = ui8Param;
}

void
SYS_DefaultDispContrastSet(uint8_t ui8Param)
{
	g_sSystem.ui8DefaultDsplContrast = ui8Param;
}


void
SYS_DefaultDispBrightnessSet(uint8_t ui8Param)
{
	g_sSystem.ui8DefaultDsplBrightness = ui8Param;
}

void
SYS_DefaultActiveTineSet(uint8_t ui8Minute, uint8_t ui8Second)
{
	 g_sSystem.sDefaultActiveTime.i8Minute = ui8Minute;
	 g_sSystem.sDefaultActiveTime.i8Second = ui8Second;
}

void
SYS_DefaultMotionDelay(uint8_t ui8Param)
{
	g_sSystem.ui8DefaultMotionDelay = ui8Param;
}



