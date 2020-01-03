/**
 * @file MP3Player.h
 * @brief
 *
 *
 */


#ifndef MP3PLAYER_H_
#define MP3PLAYER_H_

#if defined(_WIN64) || defined(_WIN32)

enum
{
	kStatus_Success,
	kStatus_Fail,
};

typedef int32_t status_t;

#else
#include "fsl_common.h"
#endif
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
void MP3_Play(char * filePath);

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
int MP3_GetPlaybackTime(void);
/**
 *
 */
void MP3_Task();

status_t MP3_ComputeSongDuration(char* path, uint32_t * seconds);

/**
 * Sets audio volume level
 */
void MP3_SetVolume(uint32_t level);

#endif /* MP3PLAYER_H_ */
