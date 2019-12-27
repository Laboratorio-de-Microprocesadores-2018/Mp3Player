/**
 * @file MP3Player.h
 * @brief
 *
 *
 */


#ifndef MP3PLAYER_H_
#define MP3PLAYER_H_
#include "fsl_common.h"

/**
 *
 */
typedef enum
{
	MP3_OK
}MP3Status;

/**
 *
 */
typedef enum
{
	MP3_RepeatAll,
	MP3_RepeatOne,
	MP3_Shuffle
}MP3PlaybackMode;

/**
 *
 */
status_t MP3_Init();

/**
 *
 */
void MP3_Play(char * dirPath, uint8_t index);

/**
 *
 */
void MP3_PlayPause();

/**
 *
 */
void MP3_Stop();


/**
 *
 */
void MP3_Next();

/**
 *
 */
void MP3_Prev();

/**
 *
 */
void MP3_Tick();

status_t MP3_ComputeSongDuration(char* path, uint32_t * seconds);

#endif /* MP3PLAYER_H_ */
