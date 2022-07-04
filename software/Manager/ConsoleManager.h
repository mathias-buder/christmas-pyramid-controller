/*
 * ConsoleManager.h
 *
 *  Created on: 08.12.2015
 *      Author: Local
 */

#ifndef I2C_MANAGER_CONSOLEMANAGER_H_
#define I2C_MANAGER_CONSOLEMANAGER_H_

#include <stdint.h>

//*****************************************************************************
//
// Defines for the command line argument parser provided as a standard part of
// TivaWare.  qs-rgb application uses the command line parser to extend
// functionality to the serial port.
//
//*****************************************************************************

#define CMDLINE_MAX_ARGS 50

//*****************************************************************************
//
//
//*****************************************************************************
typedef struct {
    char* pcParam;
    char* pcHelp;
    char* pcUsage;
    uint8_t ui8ValMin;
    uint8_t ui8ValMax;
}tCmdLineParamEntry;



//*****************************************************************************
//
// Declaration for the callback functions that will implement the command line
// functionality.  These functions get called by the command line interpreter
// when the corresponding command is typed into the command line.
//
//*****************************************************************************
extern int CMD_help (int argc, char **argv);
extern int CMD_ls (int argc, char **argv);
extern int CMD_play (int argc, char **argv);
extern int Cmd_index(int argc, char *argv[]);
//extern int CMD_xxx (int argc, char **argv);
void Dec2Bin(int value, int bitsCount, char* output);
int getarg(int* argc, char*** argv, char* arg, char** val);


#endif /* I2C_MANAGER_CONSOLEMANAGER_H_ */
