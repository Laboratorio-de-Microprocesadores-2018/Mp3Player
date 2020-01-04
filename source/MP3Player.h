/**
 * @file MP3Player.h
 * @brief
 *
 *
 */


#ifndef MP3PLAYER_H_
#define MP3PLAYER_H_


#if defined(_WIN64) || defined(_WIN32)
#include <stdint.h>
enum
{
	kStatus_Success,
	kStatus_Fail,
};

typedef int32_t status_t;

#else
#include <stdint.h>
#include "fsl_common.h"
#endif
/**
 *
 */


/* Player status */
typedef enum {
	IDLE,
	PLAYING,
	PLAYING_LAST_FRAMES,
	PAUSE_PENDING,
	PAUSE
}MP3_Status;
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
int MP3_GetMaxVolume(void);
int MP3_GetVolume();

MP3_Status MP3_GetStatus();

int MP3_GetPlaybackTime(void);
int MP3_GetTrackDuration();

#endif /* MP3PLAYER_H_ */
