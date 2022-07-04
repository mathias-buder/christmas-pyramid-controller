/*
 * AudioManager.h
 *
 *  Created on: 07.12.2015
 *      Author: Local
 */

#ifndef I2C_MANAGER_AUDIOMANAGER_H_
#define I2C_MANAGER_AUDIOMANAGER_H_



#define N_TRACK			(50)

typedef enum {
	AM_FADEIN,
	AM_FADEOUT
}AMFADE;


typedef struct {
	char*	pcPath;			// Absolut file path
	char*	pcTitle;		// Title
	char*	pcArtist;		// Artist
	char*	pcAlbum;		// Album
	int8_t	i8Duration;		// Track duration in seconds
}AMTRACK;




void AM_VolumeFade(uint8_t ui8Duration, AMFADE eType);
void AM_VolumeSet(uint8_t ui8Volume);
void AM_Play(void);
void AM_Stop(void);
void AM_Init(uint8_t ui8DefaultVolume, uint8_t ui8DefaultFadeTime, uint16_t ui16NumOfFiles);

#endif /* I2C_MANAGER_AUDIOMANAGER_H_ */
