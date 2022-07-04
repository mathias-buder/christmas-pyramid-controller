/*
 * TaskManager.h
 *
 *  Created on: 27.10.2016
 *      Author: Local
 */

#ifndef MANAGER_TASKMANAGER_H_
#define MANAGER_TASKMANAGER_H_

#include "../Driver/ACT.h"
#include "../Driver/DS3231.h"

//*****************************************************************************
//
// Defines
//
//*****************************************************************************
#define N_TASK					(25)


//*****************************************************************************
//
// Actuator channel bitmask
//
//*****************************************************************************
#define ACT_CH1_BM				0x01
#define ACT_CH2_BM				0x02
#define ACT_CH3_BM				0x04
#define ACT_CH4_BM				0x08
#define ACT_CH5_BM				0x10
#define ACT_CH6_BM				0x20
#define ACT_CH7_BM				0x40
#define ACT_CH8_BM				0x80


//*****************************************************************************
//
// Enums
//
//*****************************************************************************
typedef enum {
	STARTTIME_H,						// task start hour
	STARTTIME_M,						// task start minute
	STARTTIME_S,						// task start second
	STOPTIME_H,							// task stop hour
	STOPTIME_M,							// task stop minute
	STOPTIME_S,							// task stop second
	ACTIVETIME_H,						// task active hour
	ACTIVETIME_M,						// task active minute
	ACTIVETIME_S,						// task active second
	MOTION,								// task motion flag
	AUDIO,								// task audio flag
	ACTUATOR,							// task actuator flag
	ACTUATOR_CH,						// task actuator channel
	ACTUATOR_PROG						// task actuator program
}TASKPARAM;


//*****************************************************************************
//
// Structurs
//
//*****************************************************************************
typedef struct {
	int16_t	i16Id;						// task id
	TIME	sStartTime;					// start time (hh:mm:ss)
	TIME	sStopTime;					// stop time (hh:mm:ss)
	TIME	sActiveTime;				// motion event aktive time (hh:mm:ss)
	bool	bMotion;					// motion active flag
	bool	bAudio;						// audio active flag
	bool	bActuator;					// actuator active flag
	bool	bActuatorCH[ACT_N_CH];		// active actuator channel no.
	uint8_t	ui8ActuatorProg;			// actuator program (runns when actuators are activated)
}TASK;

extern TASK g_sTask[N_TASK];



//*****************************************************************************
//
// Function prototype declaration
//
//*****************************************************************************
void TM_Init();
uint8_t TM_TaskAdd(TIME sStartTime, TIME sStopTime, TIME sActiveTime, bool bMotion, bool bAudio, bool bActuator, uint8_t ui8ActuatorProg);
void TM_TaskSet(uint8_t ui8TaskIdxToSet, TASKPARAM eParam, uint8_t ui8Value);
void TM_TaskRemove(uint8_t ui8Idx);
void TM_TaskPrint(void);
int8_t TM_TaskSearch(int32_t i32CurrentTimeInSeconds);

#endif /* MANAGER_TASKMANAGER_H_ */
