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
#include "utils/uartstdio.h"
#include "../Driver/fatfs/ff.h"
#include "../Driver/fatfs/diskio.h"
#include "../Driver/VS10xx.h"
#include "../Driver/LED.h"
#include "FileManager.h"
#include "TaskManager.h"
#include "../Msg.h"
#include "../System.h"
#include "utils/ustdlib.h"



//*****************************************************************************
//
// Printf functions used in this file
//
//*****************************************************************************
#define FILE_ID				("FLMG")
#define eprintf(fms, ...)	do{	UARTprintf("\033[31;1m"); \
								UARTprintf("[ %s ] ERROR " fms, FILE_ID, __VA_ARGS__); \
								UARTprintf("\033[0m");} while(0)

#define dprintf(fms, ...)	UARTprintf("[ %s ] " fms, FILE_ID, __VA_ARGS__); \


//*****************************************************************************
//
// Defines the size of the buffers that hold the path, or temporary data from
// the SD card.  There are two buffers allocated of this size.  The buffer size
// must be large enough to hold the longest expected full path name, including
// the file name, and a trailing null character.
//
//*****************************************************************************
#define SD_GPIO_SYSCTL_PERIPH		SYSCTL_PERIPH_GPIOD
#define SD_GPIO_PORT_BASE			GPIO_PORTD_BASE
#define SD_CD_PIN					GPIO_PIN_2
#define SD_WP_PIN					GPIO_PIN_3
#define SD_CD_INT_PIN				GPIO_INT_PIN_2
#define SD_GPIO_PINS				(SD_CD_PIN | SD_WP_PIN)

//*****************************************************************************
//
// Defines the size of the buffers that hold the path, or temporary data from
// the SD card.  There are two buffers allocated of this size.  The buffer size
// must be large enough to hold the longest expected full path name, including
// the file name, and a trailing null character.
//
//*****************************************************************************
#define CFG_DIR_NAME				("config")
#define TASK_CFG_FILE_NAME			("task.cfg")
#define SYS_CFG_FILE_NAME			("system.cfg")

//*****************************************************************************
//
// This buffer holds the full path to the current working directory.  Initially
// it is root ("/").
//
//*****************************************************************************
char g_pcCwdBuf[PATH_BUF_SIZE] = "/";

//*****************************************************************************
//
// A temporary data buffer used when manipulating file paths, or reading data
// from the SD card.
//
//*****************************************************************************
char g_pcTmpBuf[PATH_BUF_SIZE];
char g_pcConfigBuf[PATH_BUF_SIZE];
char g_pcFileList[N_MAX_FILES][PATH_BUF_SIZE];

//*****************************************************************************
//
// The following are data structures used by FatFs.
//
//*****************************************************************************
static FATFS	g_sFatFs;
static DIR		g_sDirObject;
static FILINFO	g_sFileInfo;
static FIL		g_sFMFileObject;
static FIL		g_sConfigFileObject;	// Structure that holds the user
										// configuration data

//*****************************************************************************
//
// A structure that holds a mapping between an FRESULT numerical code, and a
// string representation.  FRESULT codes are returned from the FatFs FAT file
// system driver.
//
//*****************************************************************************
typedef struct
{
    FRESULT	iFResult;
    char*	pcResultStr;
}
tFResultString;

//*****************************************************************************
//
// A macro to make it easy to add result codes to the table.
//
//*****************************************************************************
#define FRESULT_ENTRY(f)        { (f), (#f) }

//*****************************************************************************
//
// A table that holds a mapping between the numerical FRESULT code and it's
// name as a string.  This is used for looking up error codes for printing to
// the console.
//
//*****************************************************************************
tFResultString g_psFResultStrings[] =
{
    FRESULT_ENTRY(FR_OK),
    FRESULT_ENTRY(FR_DISK_ERR),
    FRESULT_ENTRY(FR_INT_ERR),
    FRESULT_ENTRY(FR_NOT_READY),
    FRESULT_ENTRY(FR_NO_FILE),
    FRESULT_ENTRY(FR_NO_PATH),
    FRESULT_ENTRY(FR_INVALID_NAME),
    FRESULT_ENTRY(FR_DENIED),
    FRESULT_ENTRY(FR_EXIST),
    FRESULT_ENTRY(FR_INVALID_OBJECT),
    FRESULT_ENTRY(FR_WRITE_PROTECTED),
    FRESULT_ENTRY(FR_INVALID_DRIVE),
    FRESULT_ENTRY(FR_NOT_ENABLED),
    FRESULT_ENTRY(FR_NO_FILESYSTEM),
    FRESULT_ENTRY(FR_MKFS_ABORTED),
    FRESULT_ENTRY(FR_TIMEOUT),
    FRESULT_ENTRY(FR_LOCKED),
    FRESULT_ENTRY(FR_NOT_ENOUGH_CORE),
    FRESULT_ENTRY(FR_TOO_MANY_OPEN_FILES),
    FRESULT_ENTRY(FR_INVALID_PARAMETER),
};

//*****************************************************************************
//
// A macro that holds the number of result codes.
//
//*****************************************************************************
#define NUM_FRESULT_CODES       (sizeof(g_psFResultStrings) /                 \
                                 sizeof(tFResultString))


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
//*****************************************************************************
//
// This function returns a string representation of an error code that was
// returned from a function call to FatFs.  It can be used for printing human
// readable error messages.
//
//*****************************************************************************
const char *
StringFromFResult(FRESULT iFResult)
{
    uint_fast8_t ui8Idx;

    //
    // Enter a loop to search the error code table for a matching error code.
    //
    for(ui8Idx = 0; ui8Idx < NUM_FRESULT_CODES; ui8Idx++)
    {
        //
        // If a match is found, then return the string name of the error code.
        //
        if(g_psFResultStrings[ui8Idx].iFResult == iFResult)
        {
            return(g_psFResultStrings[ui8Idx].pcResultStr);
        }
    }

    //
    // At this point no matching code was found, so return a string indicating
    // an unknown error.
    //
    return("UNKNOWN ERROR CODE");
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
//*****************************************************************************
//
// This is the handler for this SysTick interrupt.  FatFs requires a timer tick
// every 10 ms for internal timing purposes.
//
//*****************************************************************************
void
SysTickHandler(void)
{
    //
    // Call the FatFs tick timer.
    //
    disk_timerproc();
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
//*****************************************************************************
//
// This function implements the "ls" command.  It opens the current directory
// and enumerates through the contents, and prints a line for each item it
// finds.  It shows details such as file attributes, time and date, and the
// file size, along with the name.  It shows a summary of file sizes at the end
// along with free space.
//
//*****************************************************************************
int
FM_ls(void)
{
    uint32_t ui32TotalSize;
    uint32_t ui32FileCount;
    uint32_t ui32DirCount;
    FRESULT iFResult;
    FATFS *psFatFs;
    char *pcFileName;
#if _USE_LFN
    char pucLfn[_MAX_LFN + 1];
    g_sFileInfo.lfname = pucLfn;
    g_sFileInfo.lfsize = sizeof(pucLfn);
#endif

    //
    // Open the current directory for access.
    //
    iFResult = f_opendir(&g_sDirObject, g_pcCwdBuf);

    //
    // Check for error and return if there is a problem.
    //
    if(iFResult != FR_OK)
    {
        return((int)iFResult);
    }

    ui32TotalSize = 0;
    ui32FileCount = 0;
    ui32DirCount = 0;

    //
    // Give an extra blank line before the listing.
    //
    UARTprintf("\n");

    //
    // Enter loop to enumerate through all directory entries.
    //
    for(;;)
    {
    	SysCtlDelay(80000);

        //
        // Read an entry from the directory.
        //
        iFResult = f_readdir(&g_sDirObject, &g_sFileInfo);

        //
        // Check for error and return if there is a problem.
        //
        if(iFResult != FR_OK)
        {
            return((int)iFResult);
        }

        //
        // If the file name is blank, then this is the end of the listing.
        //
        if(!g_sFileInfo.fname[0])
        {
            break;
        }

        //
        // If the attribue is directory, then increment the directory count.
        //
        if(g_sFileInfo.fattrib & AM_DIR)
        {
            ui32DirCount++;
        }

        //
        // Otherwise, it is a file.  Increment the file count, and add in the
        // file size to the total.
        //
        else
        {
            ui32FileCount++;
            ui32TotalSize += g_sFileInfo.fsize;
        }

#if _USE_LFN
        pcFileName = ((*g_sFileInfo.lfname)?g_sFileInfo.lfname:g_sFileInfo.fname);
#else
        pcFileName = g_sFileInfo.fname;
#endif
        //
        // Print the entry information on a single line with formatting to show
        // the attributes, date, time, size, and name.
        //
        UARTprintf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9u  %s\n",
                   (g_sFileInfo.fattrib & AM_DIR) ? 'D' : '-',
                   (g_sFileInfo.fattrib & AM_RDO) ? 'R' : '-',
                   (g_sFileInfo.fattrib & AM_HID) ? 'H' : '-',
                   (g_sFileInfo.fattrib & AM_SYS) ? 'S' : '-',
                   (g_sFileInfo.fattrib & AM_ARC) ? 'A' : '-',
                   (g_sFileInfo.fdate >> 9) + 1980,
                   (g_sFileInfo.fdate >> 5) & 15,
                   g_sFileInfo.fdate & 31,
                   (g_sFileInfo.ftime >> 11),
                   (g_sFileInfo.ftime >> 5) & 63,
                   g_sFileInfo.fsize,
                   pcFileName);
    }

    //
    // Print summary lines showing the file, dir, and size totals.
    //
    UARTprintf("\n%4u File(s),%10u bytes total\n%4u Dir(s)",
                ui32FileCount, ui32TotalSize, ui32DirCount);

    //
    // Get the free space.
    //
    iFResult = f_getfree("/", (DWORD *)&ui32TotalSize, &psFatFs);

    //
    // Check for error and return if there is a problem.
    //
    if(iFResult != FR_OK)
    {
        return((int)iFResult);
    }

    //
    // Display the amount of free space that was calculated.
    //
    UARTprintf(", %10uK bytes frei\n", (ui32TotalSize *
                                        psFatFs->free_clust / 2));

    //
    // Made it to here, return with no errors.
    //
    return(0);
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
//*****************************************************************************
//
// This function implements the "ls" command.  It opens the current directory
// and enumerates through the contents, and prints a line for each item it
// finds.  It shows details such as file attributes, time and date, and the
// file size, along with the name.  It shows a summary of file sizes at the end
// along with free space.
//
//*****************************************************************************
int
FM_Index(void)
{
	uint16_t ui16Idx;
    FRESULT iFResult;
    char *pcFileName;
#if _USE_LFN
    char pucLfn[_MAX_LFN + 1];
    g_sFileInfo.lfname = pucLfn;
    g_sFileInfo.lfsize = sizeof(pucLfn);
#endif

    //
    // Open the current directory for access.
    //
    iFResult = f_opendir(&g_sDirObject, "/");

    //
    // Check for error and return if there is a problem.
    //
    if(iFResult != FR_OK)
    {
        return((int)iFResult);
    }

    dprintf("Indiziere Dateien ...", 0);

    //
    // Enter loop to enumerate through all directory entries.
    //
    ui16Idx = 0;
    for(;;)
    {
        //
        // Read an entry from the directory.
        //
        iFResult = f_readdir(&g_sDirObject, &g_sFileInfo);

        //
        // Check for error and return if there is a problem.
        //
        if(iFResult != FR_OK)
        {
            return((int)iFResult);
        }

        //
        // If the file name is blank, then this is the end of the listing.
        //
        if(!g_sFileInfo.fname[0])
        {
            break;
        }

#if _USE_LFN
        pcFileName = ((*g_sFileInfo.lfname)?g_sFileInfo.lfname:g_sFileInfo.fname);
#else
        pcFileName = g_sFileInfo.fname;
#endif

        //
        // Add full files path to file list in case it is not hidden or an directory
        //
        if((g_sFileInfo.fattrib & AM_DIR) <= 0 && (g_sFileInfo.fattrib & AM_HID) <= 0)
        {
        	strcpy(g_pcFileList[ui16Idx++], pcFileName);

        	//
        	// Check for end of list reached
        	//
        	if(ui16Idx >= N_MAX_FILES)
        	{
        		break;
        	}

        }
    }

    OkPrint;

    //
    // Report result to user
    //
    if(ui16Idx < N_MAX_FILES)
    {
    	dprintf("%i Datein indiziert.\n", ui16Idx);
    	return ui16Idx;
    }
    else
    {
    	 dprintf("Maximale Dateianzahl (%i) bei Indizierung erreicht.\n", N_MAX_FILES);
    	 return N_MAX_FILES;
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
FM_cd(int argc, char *argv[])
{
    uint_fast8_t ui8Idx;
    FRESULT iFResult;

    //
    // Copy the current working path into a temporary buffer so it can be
    // manipulated.
    //
    strcpy(g_pcTmpBuf, g_pcCwdBuf);

    //
    // If the first character is /, then this is a fully specified path, and it
    // should just be used as-is.
    //
    if(argv[1][0] == '/')
    {
        //
        // Make sure the new path is not bigger than the cwd buffer.
        //
        if(strlen(argv[1]) + 1 > sizeof(g_pcCwdBuf))
        {
        	eprintf("Resulting path name is too long\n", 0);
            return(0);
        }

        //
        // If the new path name (in argv[1])  is not too long, then copy it
        // into the temporary buffer so it can be checked.
        //
        else
        {
            strncpy(g_pcTmpBuf, argv[1], sizeof(g_pcTmpBuf));
        }
    }

    //
    // If the argument is .. then attempt to remove the lowest level on the
    // CWD.
    //
    else if(!strcmp(argv[1], ".."))
    {
        //
        // Get the index to the last character in the current path.
        //
        ui8Idx = strlen(g_pcTmpBuf) - 1;

        //
        // Back up from the end of the path name until a separator (/) is
        // found, or until we bump up to the start of the path.
        //
        while((g_pcTmpBuf[ui8Idx] != '/') && (ui8Idx > 1))
        {
            //
            // Back up one character.
            //
            ui8Idx--;
        }

        //
        // Now we are either at the lowest level separator in the current path,
        // or at the beginning of the string (root).  So set the new end of
        // string here, effectively removing that last part of the path.
        //
        g_pcTmpBuf[ui8Idx] = 0;
    }

    //
    // Otherwise this is just a normal path name from the current directory,
    // and it needs to be appended to the current path.
    //
    else
    {

        //
        // Test to make sure that when the new additional path is added on to
        // the current path, there is room in the buffer for the full new path.
        // It needs to include a new separator, and a trailing null character.
        //
        if(strlen(g_pcTmpBuf) + strlen(argv[1]) + 1 + 1 > sizeof(g_pcCwdBuf))
        {
        	eprintf("Resulting path name is too long\n", 0);
            return(0);
        }

        //
        // The new path is okay, so add the separator and then append the new
        // directory to the path.
        //
        else
        {
            //
            // If not already at the root level, then append a /
            //
            if(strcmp(g_pcTmpBuf, "/"))
            {
                strcat(g_pcTmpBuf, "/");
            }

            //
            // Append the new directory to the path.
            //
            strcat(g_pcTmpBuf, argv[1]);
        }
    }


    //
    // At this point, a candidate new directory path is in chTmpBuf.  Try to
    // open it to make sure it is valid.
    //
    iFResult = f_opendir(&g_sDirObject, g_pcTmpBuf);

    //
    // If it can't be opened, then it is a bad path.  Inform the user and
    // return.
    //
    if(iFResult != FR_OK)
    {
    	eprintf("cd: %s\n", g_pcTmpBuf);
        return((int)iFResult);
    }

    //
    // Otherwise, it is a valid new path, so copy it into the CWD.
    //
    else
    {
        strncpy(g_pcCwdBuf, g_pcTmpBuf, sizeof(g_pcCwdBuf));
    }

    //
    // Return success.
    //
    return(0);
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
FM_pwd(int argc, char *argv[])
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
FM_cat(int argc, char *argv[])
{
    FRESULT iFResult;
    uint32_t ui32BytesRead;

    //
    // First, check to make sure that the current path (CWD), plus the file
    // name, plus a separator and trailing null, will all fit in the temporary
    // buffer that will be used to hold the file name.  The file name must be
    // fully specified, with path, to FatFs.
    //
    if(strlen(g_pcCwdBuf) + strlen(argv[1]) + 1 + 1 > sizeof(g_pcTmpBuf))
    {
    	eprintf("Resulting path name is too long\n", 0);
        return(0);
    }

    //
    // Copy the current path to the temporary buffer so it can be manipulated.
    //
    strcpy(g_pcTmpBuf, g_pcCwdBuf);

    //
    // If not already at the root level, then append a separator.
    //
    if(strcmp("/", g_pcCwdBuf))
    {
        strcat(g_pcTmpBuf, "/");
    }

    //
    // Now finally, append the file name to result in a fully specified file.
    //
    strcat(g_pcTmpBuf, argv[1]);

    //
    // Open the file for reading.
    //
    iFResult = f_open(&g_sFMFileObject, g_pcTmpBuf, FA_READ);

    //
    // If there was some problem opening the file, then return an error.
    //
    if(iFResult != FR_OK)
    {
        return((int)iFResult);
    }

    //
    // Enter a loop to repeatedly read data from the file and display it, until
    // the end of the file is reached.
    //
    do
    {
        //
        // Read a block of data from the file.  Read as much as can fit in the
        // temporary buffer, including a space for the trailing null.
        //
        iFResult = f_read(&g_sFMFileObject, g_pcTmpBuf, sizeof(g_pcTmpBuf) - 1,
                          (UINT *)&ui32BytesRead);

        //
        // If there was an error reading, then print a newline and return the
        // error to the user.
        //
        if(iFResult != FR_OK)
        {
            UARTprintf("\n");
            return((int)iFResult);
        }

        //
        // Null terminate the last block that was read to make it a null
        // terminated string that can be used with printf.
        //
        g_pcTmpBuf[ui32BytesRead] = 0;

        //
        // Print the last chunk of the file that was received.
        //
        UARTprintf("%s", g_pcTmpBuf);
    }
    while(ui32BytesRead == sizeof(g_pcTmpBuf) - 1);

    //
    // Return success.
    //
    return(0);
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
FRESULT
FM_FileOpen(char* pcFile, FIL* psFile)
{
	//
	// First, check to make sure that the current path (CWD), plus the file
	// name, plus a separator and trailing null, will all fit in the temporary
	// buffer that will be used to hold the file name.  The file name must be
	// fully specified, with path, to FatFs.
	//
	if(strlen(g_pcCwdBuf) + strlen(pcFile) + 1 + 1 > sizeof(g_pcTmpBuf))
	{
		eprintf("Resulting cPath name is too long\n", 0);
		return(FR_INVALID_NAME);
	}

	//
	// Copy the current path to the temporary buffer so it can be manipulated.
	//
	strcpy(g_pcTmpBuf, g_pcCwdBuf);

	//
	// If not already at the root level, then append a separator.
	//
	if(strcmp("/", g_pcCwdBuf))
	{
		strcat(g_pcTmpBuf, "/");
	}

	//
	// Now finally, append the file name to result in a fully specified file.
	//
	strcat(g_pcTmpBuf, pcFile);

	//
	// Open the file for reading.
	//

	/*
	FRESULT f_open (
	  FIL* fp,              [OUT] Pointer to the file object structure
	  const TCHAR* path,    [IN] File name
	  BYTE mode             [IN] Mode flags
	);
	*/
	return (f_open(psFile, g_pcTmpBuf, FA_READ));
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
FRESULT
FM_FileClose(FIL* psFile)
{
	return (f_close(psFile));
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
FM_FileDelete(char* pcFile)
{
    FRESULT iFResult;

    //
    // First, check to make sure that the current path (CWD), plus the file
    // name, plus a separator and trailing null, will all fit in the temporary
    // buffer that will be used to hold the file name.  The file name must be
    // fully specified, with path, to FatFs.
    //
    if(strlen(g_pcCwdBuf) + strlen(pcFile) + 1 + 1 > sizeof(g_pcTmpBuf))
    {
    	eprintf("Resultierender Pfadname ist zu lang.\n", 0);
        return;
    }

    //
    // Copy the current path to the temporary buffer so it can be manipulated.
    //
    strcpy(g_pcTmpBuf, g_pcCwdBuf);

    //
    // If not already at the root level, then append a separator.
    //
    if(strcmp("/", g_pcCwdBuf))
    {
        strcat(g_pcTmpBuf, "/");
    }

    //
    // Now finally, append the file name to result in a fully specified file.
    //
    strcat(g_pcTmpBuf, pcFile);

	iFResult = f_unlink(g_pcTmpBuf);

	if(iFResult != FR_OK)
	{
		eprintf("%s, Datei %s konnte nicht geloescht werden.\n", StringFromFResult(iFResult), pcFile);
		return;
	}

	dprintf("Datei %s geloescht.\n", pcFile);
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
FRESULT
FM_Save(char* pcFilePath, void* pData, uint32_t ui32SizeInByte)
{
	FRESULT iFResult;
	uint32_t ui32BytsWritten;

	//
	// Open/Create file
	//
	iFResult = f_open(&g_sConfigFileObject, pcFilePath, FA_OPEN_ALWAYS | FA_WRITE);

	//
	// Check for errors
	//
	if(iFResult != FR_OK)
	{
		eprintf("%s, Datei %s konnte nicht erstellt/geoeffnet werden.\n", StringFromFResult(iFResult), pcFilePath);
		return iFResult;
	}

	//
	// Write file
	//
	iFResult = f_write(&g_sConfigFileObject, pData, ui32SizeInByte, &ui32BytsWritten);

	//
	// Check for any errors
	//
	if(iFResult != FR_OK)
	{
		eprintf("%s, Fehler beim Schreiben in Datei %s\n", StringFromFResult(iFResult), pcFilePath);
		return iFResult;
	}

	//
	// Close file
	//
	iFResult = f_close(&g_sConfigFileObject);

	//
	// Check for errors
	//
	if(iFResult != FR_OK)
	{
		eprintf("%s, Datei %s konnte nicht geschlossen werden.\n", StringFromFResult(iFResult), pcFilePath);
		return iFResult;
	}

	return iFResult;
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
FRESULT
FM_Load(char* pcFilePath, void* pData, uint32_t ui32SizeInByte)
{
	FRESULT iFResult;
	uint32_t ui32BytsRead;

	//
	// Open configuration file
	//
	iFResult = f_open(&g_sConfigFileObject, pcFilePath, FA_OPEN_EXISTING | FA_READ);

	//
	// Check for errors
	//
	if(iFResult != FR_OK)
	{
		eprintf("%s, Datei %s konnte nicht geoeffnet werden.\n", StringFromFResult(iFResult), pcFilePath);
		return iFResult;
	}

	//
	// Read configuration file
	//
	iFResult = f_read(&g_sConfigFileObject, pData, ui32SizeInByte, &ui32BytsRead);

	//
	// Check for any errors
	//
	if(iFResult != FR_OK)
	{
		eprintf("%s beim Schreiben in Datei %s\n", StringFromFResult(iFResult), pcFilePath);
		return iFResult;
	}

	//
	// Close configuration file
	//
	iFResult = f_close(&g_sConfigFileObject);

	//
	// Check for errors
	//
	if(iFResult != FR_OK)
	{
		eprintf("%s, Datei %s konnte nicht geschlossen werden.\n", StringFromFResult(iFResult), pcFilePath);
		return iFResult;
	}

	return iFResult;
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
FM_ConfigSave(FCFGTYPE eType)
{
	FRESULT iFResult;
	FILINFO iInfo;



	//
	// Check if "config" directory is present
	//
	if(f_stat(CFG_DIR_NAME, &iInfo) != FR_OK)
	{
		dprintf("Erstelle Konfigurationsordner %s ...\n", CFG_DIR_NAME);

		iFResult = f_mkdir(CFG_DIR_NAME);

		if(iFResult != FR_OK)
		{
			eprintf("%s, Konfigurationsordner %s konnte nicht erstellt werden.\n", StringFromFResult(iFResult), CFG_DIR_NAME);
			return;
		}
	}

	switch(eType)
	{
		case F_TASK:

			dprintf("Speichere Aufgabenkonfiguration ...\n", 0);

			usprintf(g_pcConfigBuf, "%s/%s", CFG_DIR_NAME, TASK_CFG_FILE_NAME);

			if(FM_Save(g_pcConfigBuf, &g_sTask, sizeof(g_sTask)) != FR_OK)
			{
				eprintf("Aufgaben konnten nicht in Datei %s gespeichert werden\n", g_pcConfigBuf);
				return;
			}

			dprintf("Aufgabenkonfiguration gespeichert\n", 0);

			break;

		case F_SYS:

			dprintf("Speichere Systemnkonfiguration ...\n", 0)

			usprintf(g_pcConfigBuf, "%s/%s", CFG_DIR_NAME, SYS_CFG_FILE_NAME);

			if(FM_Save(g_pcConfigBuf, &g_sSystem, sizeof(g_sSystem)) != FR_OK)
			{
				eprintf("Systemkonfiguration konnte nicht gespeichert werden\n", g_pcConfigBuf);
				return;
			}

			dprintf("Systemkonfiguration gespeichert\n", 0);

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
FM_ConfigLoad(void)
{


	//
	// Load system default configuration file
	//
	dprintf("Lade Systemkonfiguration ...", 0);

	usprintf(g_pcConfigBuf, "%s/%s", CFG_DIR_NAME, SYS_CFG_FILE_NAME);
	if(FM_Load(g_pcConfigBuf, &g_sSystem, sizeof(g_sSystem)) == FR_OK)
	{
		OkPrint;
	}
	//
	// Load task configuration file
	//
	dprintf("Lade Aufgabenkonfiguration ...", 0);

	usprintf(g_pcConfigBuf, "%s/%s", CFG_DIR_NAME, TASK_CFG_FILE_NAME);
	if(FM_Load(g_pcConfigBuf, &g_sTask, sizeof(g_sTask)) == FR_OK)
	{
		OkPrint;
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
FRESULT
FM_Mount(void)
{
    return (f_mount(0, &g_sFatFs));
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
FRESULT
FM_SDCheck(void)
{
	FRESULT iFResult;

	dprintf("Suche nach SD-Karte ...\n", 0);

	//
	// Check SD detect pin
	//
	if(!GPIOPinRead(SD_GPIO_PORT_BASE, SD_CD_PIN))
	{
		dprintf("SD-Karte gefunden.\n", 0);
		dprintf("Aktiviere Dateisystem ...", 0);

		//
		// Mount file system on SD card
		//
		iFResult = FM_Mount();

		//
		// Check for any errors
		//
		if(iFResult != FR_OK)
		{
			eprintf("%s", StringFromFResult(iFResult));
			return iFResult;
		}


		//
		// Enable SD detect LED
		//
		LEDEnable(LED_SD_DETECT);
		OkPrint;

		//
		// Index files
 		//
		FM_Index();

		return iFResult;
	}

	//
	// Disable SD detect LED and inform user
	//
	LEDDisable(LED_SD_DETECT);
	dprintf("Keine SD Card gefunden.\n", 0);

	return iFResult;
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
bool
FM_SDProtected(void)
{
	if(!GPIOPinRead(SD_GPIO_PORT_BASE, SD_WP_PIN))
	{
		dprintf("SD Card ist schreibgeschützt.\n", 0);
		return true;
	}
	else
	{
		dprintf("SD Card ist nicht schreibgeschützt.\n", 0);
		return false;
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
FM_Init(void)
{
	//
	// Enable the peripheral
	//
    SysCtlPeripheralEnable(SD_GPIO_SYSCTL_PERIPH);

	//
	// Set coresponding pins to be a GPIO input
	//
    GPIOPinTypeGPIOInput(SD_GPIO_PORT_BASE, SD_GPIO_PINS);
    GPIOPadConfigSet(SD_GPIO_PORT_BASE, SD_GPIO_PINS, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntTypeSet(SD_GPIO_PORT_BASE, SD_CD_INT_PIN, GPIO_BOTH_EDGES);
    GPIOIntEnable(SD_GPIO_PORT_BASE, SD_CD_INT_PIN);

	//
    // Configure SysTick for a 100Hz interrupt. The FatFs driver wants a 10 ms tick.
    //
    ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / 100);
    ROM_SysTickEnable();
    ROM_SysTickIntEnable();
}

