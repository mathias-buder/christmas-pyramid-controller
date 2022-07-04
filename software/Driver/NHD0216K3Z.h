/*
 * NHD0216K3Z.h
 *
 *  Created on: 11.12.2015
 *      Author: mbuder
 */

#ifndef I2C_DRIVER_NHD0216K3Z_H_
#define I2C_DRIVER_NHD0216K3Z_H_

#include <stdint.h>

/*******************************************
 * NHD0216K3Z protocol definition
 *******************************************/
#define NHD0216K3Z_COMMAND_SIZE		3
#define NHD0216K3Z_PREFIX			0
#define NHD0216K3Z_COMMAND			1
#define NHD0216K3Z_VAL				2


/*******************************************
 * NHD0216K3Z Commands
 *******************************************/
#define NHD0216K3Z_COMMAND_PREFIX		0xFE
#define NHD0216K3Z_ON					0x41
#define NHD0216K3Z_OFF					0x42
#define NHD0216K3Z_CURSOR_SET			0x45
#define NHD0216K3Z_CURSOR_HOME			0x46
#define NHD0216K3Z_CURSOR_BLINK_ON		0x4B
#define NHD0216K3Z_CURSOR_BLINK_OFF		0x4C
#define NHD0216K3Z_ULINE_ON				0x47
#define NHD0216K3Z_ULINE_OFF			0x48
#define NHD0216K3Z_MOVE_LEFT			0x49
#define NHD0216K3Z_MOVE_RIGHT			0x49
#define NHD0216K3Z_BACKSPACE			0x4E
#define NHD0216K3Z_CLEAR_SCREEN			0x51
#define NHD0216K3Z_CONTRAST_SET			0x52
#define NHD0216K3Z_BRIGHTNESS_SET		0x53
#define NHD0216K3Z_BAUD_SET				0x61
/*******************************************
 * NHD0216K3Z Properties
 *******************************************/
#define NHD0216K3_ROWS					2
#define NHD0216K3_ROW_2_ADDR			0x40
#define NHD0216K3_COLUMNS				16

enum{
	LEFT,
	RIGHT,
	CENTER
}NHD0216K3Z_ALIGEMENT;

/*******************************************
 * NHD0216K3Z Prototype declaration
 *******************************************/
void NHD0216K3ZInit(uint8_t	ui8Contrast, uint8_t ui8Brightness);
void NHD0216K3ZSendData(uint8_t *pui8Buffer, uint32_t ui32Count);
void NHD0216K3ZSendCommand(uint8_t ui8Command, int8_t i8Val);
void NHD0216K3ZPrintText(uint8_t* pui8Data, uint8_t ui8Row, uint8_t ui8Aligment);
void NHD0216K3ZOn(void);
void NHD0216K3ZOff(void);
void NHD0216K3ZSetBrightness(uint8_t ui8Brightness);
void NHD0216K3ZSetContrast(uint8_t ui8Contrast);

#endif /* I2C_DRIVER_NHD0216K3Z_H_ */
