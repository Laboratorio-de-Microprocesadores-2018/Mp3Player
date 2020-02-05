/**
 * @file MP3Player.h
 * @brief
 *
 *
 */


#ifndef MP3PLAYER_H_
#define MP3PLAYER_H_

#include <stdint.h>

#if defined(_WIN64) || defined(_WIN32)
#ifndef kStatus_Success
#define kStatus_Success 0
#endif
#ifndef kStatus_Fail
#define kStatus_Fail -1
#endif


#define status_t int32_t
#else
#include "fsl_common.h"
#endif



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
status_t MP3_Init(void);
void MP3_Deinit(void);

/**
 *
 */
void MP3_Task(void);

/**
 *
 */
void MP3_SetSongsQueue(uint32_t* songIndexs, uint32_t nSongs);

/**
 *
 */
void MP3_Play(char* dirPath, uint32_t index);

/**
 *
 */
void MP3_Next(void);

/**
 *
 */
void MP3_Prev(void);

/**
 *
 */
void MP3_PauseResume(void);

/**
 *
 */
void MP3_Stop(void);


/**
 *
 */
uint32_t MP3_GetPlaybackTime(void);



status_t MP3_ComputeSongDuration(char* path, uint32_t * seconds);

/**
 * @brief Get current song duration
 */
uint32_t MP3_GetTrackDuration(void);


MP3_Status MP3_GetStatus(void);

/**
 * Sets audio volume level
 */
void MP3_SetVolume(uint32_t level);
int MP3_GetVolume(void);
int MP3_GetMaxVolume(void);

/**
 *
 */
void MP3_SetTrackChangedCB(void(*callback)(char* filename));

/**
 *
 */
uint32_t MP3_GetSongNumber(void);

/**
 *
 */
uint32_t MP3_GetQueueLength(void);


#endif /* MP3PLAYER_H_ */
