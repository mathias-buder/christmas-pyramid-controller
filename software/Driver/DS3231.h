/*
 * DS3231.h
 *
 *  Created on: 12.08.2015
 *      Author: Mathias
 */

#ifndef RTC_TEST_DRIVERS_DS3231_H_
#define RTC_TEST_DRIVERS_DS3231_H_

//*****************************************************************************
//
// DS3231 Constants
//
//*****************************************************************************
//#define DS3231_N_DATA					7
#define DS3231_N_REG            		19	// Number of available registers
#define DS3231_TEMP_DATA				2	// Number of byts to be read for temperature

/*
#define SECOND							0
#define MINUTE							1
#define HOUR							2
#define DAY								3
#define DATE							4
#define MONTH							5
#define YEAR							6
*/
//*****************************************************************************
//
// DS3231 Register addresses
//
//*****************************************************************************
#define DS3231_SECONDS					0x00
#define DS3231_MINUTES					0x01
#define DS3231_HOUR						0x02
#define DS3231_DAY						0x03
#define DS3231_DATE						0x04
#define DS3231_MONTH					0x05
#define DS3231_YEAR						0x06

#define DS3231_ALARM_1_SECONDS			0x07
#define DS3231_ALARM_1_MINUTES			0x08
#define DS3231_ALARM_1_HOURS			0x09
#define DS3231_ALARM_1_DAY				0x0A
#define DS3231_ALARM_1_DATE				0x0A
#define DS3231_ALARM_2_MINUTES			0x0B
#define DS3231_ALARM_2_HOURS			0x0C
#define DS3231_ALARM_2_DAY				0x0D
#define DS3231_ALARM_2_DATE				0x0D

#define DS3231_CTL						0x0E
#define DS3231_CTL_STATUS				0x0F
#define DS3231_AGING_OFFSET				0x10
#define DS3231_TEMP_MSB					0x11
#define DS3231_TEMP_LSB					0x12

//*****************************************************************************
//
// DS3231 Control Register (0x0E)
//
//*****************************************************************************
#define DS3231_CTL_A1IE					0x01
#define DS3231_CTL_A2IE					0x02
#define DS3231_CTL_INTCN				0x04
#define DS3231_CTL_RS1					0x08
#define DS3231_CTL_RS2					0x10
#define DS3231_CTL_CONV					0x20
#define DS3231_CTL_BBSQW				0x40
#define DS3231_CTL_EOSC					0x80

//*****************************************************************************
// Alam 1/2 Mask Bit
//*****************************************************************************
#define DS3231_ALARM_AxMx				0x80

//*****************************************************************************
// I2C Addresses for the DS3231 Real-Time Clock
//*****************************************************************************
#define DS3231_I2C_ADDRESS				0x68

//*****************************************************************************
// Helper macros
//*****************************************************************************
#define TIME2SECONDS(HH, MM, SS)		((HH * 3600) + (MM * 60) + SS)

//*****************************************************************************
// Structures
//*****************************************************************************
typedef struct {
	int8_t i8Hour;
	int8_t i8Minute;
	int8_t i8Second;
	int32_t i32TimeInSeconds;
}TIME;


typedef struct {
	uint8_t ui8Second;
	uint8_t ui8Minute;
	uint8_t ui8Hour;
	uint8_t ui8DayOfWeek;
	uint8_t ui8Day;
	uint8_t ui8Month;
	uint8_t ui8Year;
	int32_t i32TimeInSeconds;
}DS3231;

extern DS3231 g_sDS3231Data;

//*****************************************************************************
//
// Enumaration
//
//*****************************************************************************
enum
{
	DS3231_ERROR,
	DS3231_OK
};

//*****************************************************************************
//
// Prototypes APIs.
//
//*****************************************************************************
bool DS3231TimeSet(DS3231* sDate);
bool DS3231DaySet(uint8_t ui8Data);
bool DS3231DateSet(uint8_t uiData);
bool DS3231MonthSet(uint8_t uiData);
bool DS3231YearSet(uint8_t uiData);
bool DS3231HourSet(uint8_t uiData);
bool DS3231MinuteSet(uint8_t uiData);
bool DS3231SecondSet(uint8_t uiData);
DS3231* DS3231TimeGet(void);
bool DS3231TemperatureGet(float *pfData);
bool DS3231TemperatureOffsetSet(int8_t i8Data);
bool DS3231RegisterWriteSingle(uint8_t ui8Register, uint8_t pui8Data);
bool DS3231RegisterSetSingle(uint8_t ui8Register, uint8_t ui8Data);
bool DS3231RegisterWriteBurst(uint8_t ui8Register, uint8_t *pui8Data, uint8_t ui8Size);
bool DS3231RegisterReadSingle(uint8_t ui8Register, uint8_t *pui8Data);
bool DS3231RegisterReadBurst(uint8_t ui8Register, uint8_t *pui8Data, uint8_t ui8Size);
bool DS3231AllRegisterGet(uint8_t *ui8Register, uint8_t ui8Size);
uint8_t Bcd2Dec(uint8_t ui8Val);
uint8_t Dec2Bcd(uint8_t ui8Val);
void DS3231IntHandler(void);
bool DS3231Init(void);

#endif /* RTC_TEST_DRIVERS_DS3231_H_ */
