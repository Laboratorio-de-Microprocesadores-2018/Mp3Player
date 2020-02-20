/**
 * @file MP3Player.h
 * @brief
 *
 *
 */


#ifndef MP3PLAYER_H_
#define MP3PLAYER_H_

#include <stdint.h>
#include <stdbool.h>
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
	PARSING_METADATA,
	PLAYING,
	PLAYING_LAST_FRAMES,
	PAUSE,
	MP3_FASTFORWARD,
	MP3_REWIND,
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

typedef enum
{
	MP3_LOW,
	MP3_LOW_MID,
	MP3_MID,
	MP3_MID_HIGH,
	MP3_HIGH,
	MP3_NUM_BANDS,

}MP3_EqBand_t;

typedef union
{
	struct {
		int8_t low;
		int8_t lowMid;
		int8_t mid;
		int8_t midHigh;
		int8_t high;
	};
	int8_t band[MP3_NUM_BANDS];
}MP3_EqLevels_t;

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
void MP3_Resume(void);

/**
 *
 */
void MP3_Stop(void);


/**
 *
 */
void MP3_Rewind(void);

/**
 *
 */
void MP3_Fastforward(void);

void MP3_EqualizerEnable(bool b);
void MP3_GetEqualizerLevels(MP3_EqLevels_t * eqLevels);
void MP3_GetEqualizerLevelLimits(int8_t * min, int8_t * max);
void MP3_SetEqualizerLevel(MP3_EqBand_t band, int8_t level);
bool MP3_IsEqualizerEnable(void);

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
void MP3_SetVolume(uint8_t level);
uint8_t MP3_GetVolume(void);
uint8_t MP3_GetMaxVolume(void);

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
