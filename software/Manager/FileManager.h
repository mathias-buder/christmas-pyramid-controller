/*
 * FileManager.h
 *
 *  Created on: 08.12.2015
 *      Author: Local
 */

#ifndef I2C_MANAGER_FILEMANAGER_H_
#define I2C_MANAGER_FILEMANAGER_H_

#include "TaskManager.h"

#define PATH_BUF_SIZE           	80
#define N_MAX_FILES					150
#define TASK_CFG_FILE_BM			0x01
#define SYSTEM_CFG_FILE_BM			0x02



typedef enum {
	F_TASK,
	F_SYS
}FCFGTYPE;





extern char g_pcCwdBuf[PATH_BUF_SIZE];
extern char g_pcFileList[N_MAX_FILES][PATH_BUF_SIZE];



void FM_Init(void);
int FM_ls(void);
int FM_cat(int argc, char *argv[]);
int FM_cd(int argc, char *argv[]);
int FM_play(char *cPath);
int FM_Index(void);
FRESULT FM_Mount(void);
bool FM_SDInserted(void);
bool FM_SDProtected(void);
char* FM_GetPath(char *cPath);
FRESULT FM_SDCheck(void);
FRESULT FM_FileOpen(char* pcFile, FIL* psFile);
FRESULT FM_FileClose(FIL* psFile);
void FM_FileDelete(char* pcFilePath);
FRESULT FM_Save(char* pcFilePath, void* pData, uint32_t ui32SizeInByte);
FRESULT FM_Load(char* pcFilePath, void* pData, uint32_t ui32SizeInByte);
void FM_ConfigSave(FCFGTYPE eType);
void FM_ConfigLoad(void);
#endif /* I2C_MANAGER_FILEMANAGER_H_ */
