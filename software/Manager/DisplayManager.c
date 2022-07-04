/*
 * DisplayManager.c
 *
 *  Created on: 19.10.2016
 *      Author: Local
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "driverlib/sysctl.h"
#include "utils/ustdlib.h"
#include "DisplayManager.h"
#include "../Driver/NHD0216K3Z.h"
#include "../Driver/DS3231.h"




uint8_t g_ui8DisplayBuffer[16] = {0};

char* cDayName[] = {"Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"};
float fTemperature;

void
DM_UpdateDisplay(void)
{
	usprintf((char*) g_ui8DisplayBuffer, "%s %02i.%02i.%02i", cDayName[g_sDS3231Data.ui8DayOfWeek - 1], g_sDS3231Data.ui8Day, g_sDS3231Data.ui8Month, g_sDS3231Data.ui8Year);
	NHD0216K3ZPrintText(g_ui8DisplayBuffer, 1, CENTER);

	usprintf((char*) g_ui8DisplayBuffer, "%02i:%02i:%02i", g_sDS3231Data.ui8Hour, g_sDS3231Data.ui8Minute, g_sDS3231Data.ui8Second);
	NHD0216K3ZPrintText(g_ui8DisplayBuffer, 2, CENTER);



}

void
DM_Init(uint8_t	ui8Contrast, uint8_t ui8Brightness)
{
    //
	// Initialize NHD0216K3Z display
	//
	SysCtlDelay(5000000);							// Wait for the display to get ready
    NHD0216K3ZInit(ui8Contrast, ui8Brightness);
    //NHD0216K3ZPrintText("PYCO v1.0", 1, CENTER);
    //NHD0216K3ZPrintText("B.ENGINEERING", 2, CENTER);
    //SysCtlDelay(10000000);
}
