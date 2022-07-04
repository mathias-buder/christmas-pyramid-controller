/*
 * System.h
 *
 *  Created on: 06.11.2016
 *      Author: Local
 */



#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "Driver/DS3231.h"



typedef enum {
	DEF_MODE,
	DEF_VOLUME,
	DEF_FADETIME,
	AUTOSAVE,
	DEF_DISPCON,
	DEF_BRIGH,
	DEF_ACTTIME_H,
	DEF_ACTTIME_M
}SYSPARAM;



typedef struct {
	uint8_t	ui8Mode;					// Switch between 0: Manuel, 1: Auto mode
	uint8_t	ui8DefaultVolume;			// Default volume
	uint8_t	ui8DefaultFadeTime;			// Default fade-in/out time (s)
	bool	bAutoSave;					// Enable/Disable auto-save-mode
	uint8_t	ui8DefaultDsplContrast;		// Default display contrast
	uint8_t	ui8DefaultDsplBrightness;	// Default display brightness
	TIME	sDefaultActiveTime;			// Default active time
	uint8_t	ui8DefaultMotionDelay;		// Default motion delay (s)
}SYSTEM;

extern SYSTEM g_sSystem;

void SYS_ParamsPrint(void);
void SYS_ModusSet(uint8_t ui8Param);
void SYS_DefaultVolumeSet(uint8_t ui8Param);
void SYS_DefaulFadeTime(uint8_t ui8Param);
void SYS_AutosaveSet(uint8_t ui8Param);
void SYS_DefaultDispContrastSet(uint8_t ui8Param);
void SYS_DefaultDispBrightnessSet(uint8_t ui8Param);
void SYS_DefaultActiveTineSet(uint8_t ui8Minute, uint8_t ui8Second);
void SYS_DefaultMotionDelay(uint8_t ui8Param);

#endif /* SYSTEM_H_ */
