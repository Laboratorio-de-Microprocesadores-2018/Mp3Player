/**
 * @file MP3Player.c
 * @brief
 *
 *
 */
#include "MP3Player.h"
#include "FileExplorer.h"

#if defined(_WIN64) || defined(_WIN32)
#include "SDL.h"
#include "SDL_mixer.h"

#else

#include "Audio.h"
#include "mp3dec.h"
#include "Vumeter.h"

#include "math.h"
#include "assert.h"

#include "fsl_debug_console.h"
#endif


#define  READ_BUFFER_SIZE  (1024*8)

/* Every 3 frames the vumeter is updated */
#define VUMETER_UPDATE_MODULO 3




static MP3_Status status;
MP3PlaybackMode playbackMode;

/**
 * Callback called every time a new track starts playing
 */
void(*trackChangedCB)(char* filename);

#if defined(_WIN64) || defined(_WIN32)
static uint32_t 				playbackTime;
static uint32_t					startPlayingTime;
static Mix_Music* music;
static FIL 					currentFile;
static FILINFO 				currentFileInfo;
static uint32_t 			songsQueue[MAX_FILES_PER_DIR];
static uint32_t 			queueLength;
static uint32_t 			curSong;
static char 				curPath[256];

static void 				MP3_PlayCurrentSong();


#else

/* Decoder*/
static HMP3Decoder mp3Decoder;

static MP3FrameInfo mp3FrameInfo;


static FIL 					currentFile;
static FILINFO 				currentFileInfo;
//static DIR 				currentDir;
static char 				curPath[256];
//static FE_FILE_SORT_TYPE 	plSortCriteria = ABC;	// Playlist sort criteria
static uint8_t 				songsQueue[MAX_FILES_PER_DIR];
static uint8_t 				queueLength;
static uint8_t 				curSong;


static uint8_t 				readBuf[READ_BUFFER_SIZE];
static uint8_t * 			readPtr;
static uint32_t 			bytesLeft;

static int16_t 				audioBuf[MAX_SAMPLES_PER_FRAME];

static uint32_t 			frameCounter;
static float 				playbackTime;



static status_t 			MP3_FillReadBuffer(FIL * fp, uint8_t *readBuf, uint8_t *readPtr, uint16_t bytesLeft, uint32_t * nRead);
static status_t 			MP3_DecodeFrame();
static void 				MP3_PlayCurrentSong();

#endif



#if defined(_WIN64) || defined(_WIN32)
void SDL_musicFinished(void)
{
	MP3_Next();
}
#endif

status_t MP3_Init()
{
#if defined(_WIN64) || defined(_WIN32)

	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		printf("Failed to init SDL audio\n");
		return -1;
	}

	int result = 0;
	int flags = MIX_INIT_MP3;

	if (flags != (result = Mix_Init(flags))) {
		printf("Could not initialize mixer (result: %d).\n", result);
		printf("Mix_Init: %s\n", Mix_GetError());
		return -1;
	}

	if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 4096) == -1)
	{
		printf("Mix_OpenAudio: %s\n", Mix_GetError());
		return -1;
	}

	Mix_HookMusicFinished(SDL_musicFinished);

	MP3_SetVolume(MP3_GetMaxVolume() / 2);
#else
	mp3Decoder = MP3InitDecoder();

	if(mp3Decoder == 0)
	{
		// This means the memory allocation failed. This typically happens if there is not
		// enough heap memory.
		assert(mp3Decoder);
		return kStatus_Fail;
	}

	/* Initialize audio module. */
	Audio_Init();

	/* Initialize vumeter module. */
	Vumeter_Init();

	status = IDLE;

	return kStatus_Success;
#endif
}

void MP3_SetSongsQueue(uint32_t* songIndexs, uint32_t nSongs)
{
	memcpy(songsQueue, songIndexs, nSongs * sizeof(songsQueue[0]));
	queueLength = nSongs;
}


void MP3_Play(char* dirPath, uint32_t index)
{

	if (status == PLAYING)
		MP3_Stop();

	if (strcmp(curPath, dirPath) != 0)
	{
		/* Store new path. */
		strcpy(curPath, dirPath);
	}

	curSong = index;

	MP3_PlayCurrentSong();

}

static void MP3_PlayCurrentSong()
{
#if defined(_WIN64) || defined(_WIN32)

	FILINFO de;
	FE_GetFileN(curPath, songsQueue[curSong], &de);

	char filePath[255];
	sprintf(filePath, "%s\\%s", curPath, FE_ENTRY_NAME(&de));

	music = Mix_LoadMUS(filePath);

	if (music == NULL)
	{
		printf("Error loading %s! Error %s\n", filePath, Mix_GetError());
	}
	else
	{
		Mix_PlayMusic(music, 1);
		status = PLAYING;
		startPlayingTime = SDL_GetTicks();
	}

	/** Notify new track started playing. */
	trackChangedCB(FE_ENTRY_NAME(&de));

#else
	// Open current song
	FRESULT result = FE_OpenFileN(  curPath,
									songsQueue[curSong],
									&currentFileInfo,
									&currentFile,
									FA_READ);

	if(result == FR_OK)
	{
		PRINTF("Playing '%s' \n",currentFileInfo.fname);

		memset(audioBuf,0,MAX_SAMPLES_PER_FRAME);

		Audio_PushFrame(audioBuf,MAX_SAMPLES_PER_FRAME,44100,0);
		Audio_PushFrame(audioBuf,MAX_SAMPLES_PER_FRAME,44100,0);
		Audio_PushFrame(audioBuf,MAX_SAMPLES_PER_FRAME,44100,0);

		Audio_Play();

		readPtr = readBuf;
		bytesLeft = 0;
		frameCounter = 0;
		playbackTime = 0;

		status = PLAYING;
	}

	/** Notify new track started playing. */
	trackChangedCB(FE_ENTRY_NAME(&currentFileInfo));

#endif

}


void MP3_Stop()
{
#if defined(_WIN64) || defined(_WIN32)
	//Mix_PauseMusic();
	Mix_FreeMusic(music);
#else
	Audio_Stop();
	Vumeter_Clear();
	FE_CloseFile(&currentFile);
#endif
	status = IDLE;
}

void MP3_Next()
{
	if (status != IDLE)
		MP3_Stop();

	// Move to next song in current folder
	curSong = (curSong + 1) % queueLength;

	MP3_PlayCurrentSong();
}

void MP3_Prev()
{
	if(status != IDLE)
		MP3_Stop();

	if (playbackTime < 5)
		curSong = (curSong + queueLength - 1) % queueLength;

	MP3_PlayCurrentSong();
}

void MP3_PauseResume()
{
	if(status==PLAYING)
	{
#if defined(_WIN64) || defined(_WIN32)
		Mix_PauseMusic();
#else
		Audio_Pause();
#endif
		status = PAUSE;
		printf("Paused\n");
	}
	else if(status == PAUSE)
	{
#if defined(_WIN64) || defined(_WIN32)
		Mix_ResumeMusic();
#else
		Audio_Resume();
#endif
		status = PLAYING;
		printf("Playing '%s' \n",FE_ENTRY_NAME(&currentFileInfo));

	}

}

int MP3_GetPlaybackTime(void)
{
	return playbackTime;
}

uint32_t MP3_GetTrackDuration()
{
	return 334;
}

MP3_Status MP3_GetStatus()
{
	return status;
}

void MP3_SetVolume(uint32_t level)
{
#if defined(_WIN64) || defined(_WIN32)
	Mix_VolumeMusic(level);
#else
	//TODO!! 
#endif
}

int MP3_GetMaxVolume()
{
#if defined(_WIN64) || defined(_WIN32)
	return MIX_MAX_VOLUME/4;
#else
	return 32; //TODO!! 
#endif
}

int MP3_GetVolume(void)
{
#if defined(_WIN64) || defined(_WIN32)
	return Mix_VolumeMusic(-1);
#else
	return 0; //TODO!! 
#endif
}

void MP3_Task()
{
#if defined(_WIN64) || defined(_WIN32)


	switch (status)
	{
	case IDLE:

		break;

	case PLAYING:
		playbackTime = (SDL_GetTicks() - startPlayingTime) / 1000;

		break;

	case PLAYING_LAST_FRAMES:
		// When empties stop playback and close file
		FE_CloseFile(&currentFile);
		MP3_Next();
		break;

	case PAUSE_PENDING:

		break;

	case PAUSE:

		break;
}
#else
	switch(status)
	{
	case IDLE:

		break;

	case PLAYING:

		// Decode as many frames as possible
		while(Audio_QueueIsFree())
		{
			status_t s = MP3_DecodeFrame();
			if(s==kStatus_Success)
			{
				if(Vumeter_BackBufferEmpty()  &&  frameCounter%VUMETER_UPDATE_MODULO == 0)
					Vumeter_Generate(audioBuf);

				Audio_PushFrame(audioBuf,
									 mp3FrameInfo.outputSamps,
									 mp3FrameInfo.samprate,
									 frameCounter++);

				playbackTime += mp3FrameInfo.outputSamps / ((float)mp3FrameInfo.samprate * 2);

			}
			else if(s == kStatus_OutOfRange)
			{
				status = PLAYING_LAST_FRAMES;
				break;
			}
			else if(s == kStatus_Fail)
			{
				MP3_Stop();
				break;
			}
		}

		if(Audio_GetCurrentFrameNumber()%VUMETER_UPDATE_MODULO == 0)
			Vumeter_Display();

		break;

	case PLAYING_LAST_FRAMES:
		// Wait to audio buffer empties
		if(Audio_QueueIsEmpty())
		{
			// When empties stop playback and close file
			Audio_Stop();
			FE_CloseFile(&currentFile);
			MP3_Next();
		}

		break;

	case PAUSE_PENDING:

		break;

	case PAUSE:

		break;
	}
#endif
}
/**
 *    @brief
 */
static status_t MP3_FillReadBuffer(FIL * fp, uint8_t *readBuf, uint8_t *readPtr, uint16_t bytesLeft, uint32_t * nRead)
{
#if defined(_WIN64) || defined(_WIN32)

#else
	/* Move the left bytes from the end to the front */
	memmove(readBuf,readPtr,bytesLeft);

	/* Read a maximum of bytesLeft bytes from current file */
    FRESULT res = FE_ReadFile(fp, (void *)(readBuf+bytesLeft), READ_BUFFER_SIZE-bytesLeft, nRead);

    if(res == FR_OK)
    {
    	/* Zero-pad to avoid finding false sync word after last frame (from old data in readBuf) */
		if ( (*nRead) < (READ_BUFFER_SIZE - bytesLeft) )
			memset(readBuf+bytesLeft+(*nRead), 0, READ_BUFFER_SIZE-bytesLeft-(*nRead));

		return kStatus_Success;
    }

    return kStatus_Fail;

#endif
}

static status_t MP3_DecodeFrame()
{
#if defined(_WIN64) || defined(_WIN32)

#else
    uint8_t wordAlign = 0;
	bool frameDecoded = 0;
    uint32_t nRead = 0;
    status_t s = kStatus_Success;

    while (frameDecoded==false && FE_EOF(&currentFile)==false )
	{
		/* Condition to refill read buffer - should always be enough for a full frame */
		if (bytesLeft < 2*MAINBUF_SIZE) //&& !eofReached)
		{
			/* Align to 4 bytes */
			//wordAlign = (4-(bytesLeft&3)) & 3;

			/* Fill read buffer */
			s = MP3_FillReadBuffer(&currentFile,readBuf, readPtr, bytesLeft, &nRead);

			if(s==kStatus_Fail)
				break;

			bytesLeft += nRead;
			readPtr = readBuf;

		}

		/* Find start of next MP3 frame - assume EOF if no sync found */
		int32_t offset = MP3FindSyncWord(readPtr, bytesLeft);

		if (offset < 0)
		{
			readPtr = readBuf;
			bytesLeft = 0;
			continue;
		}

		readPtr += offset;
		bytesLeft -= offset;

		/* ESTO NO SE QUE ES
		//simple check for valid header
		if(((*(readPtr+1) & 24) == 8) || ((*(readPtr+1) & 6) != 2) || ((*(readPtr+2) & 240) == 240) || ((*(readPtr+2) & 12) == 12) || ((*(readPtr+3) & 3) == 2))
		{
			readPtr += 1;		//header not valid, try next one
			bytesLeft -= 1;
			continue;
		}
		*/

		switch (MP3Decode(mp3Decoder, &readPtr, (int*)&bytesLeft, audioBuf, 0))
		{
		case ERR_MP3_NONE:
			MP3GetLastFrameInfo(mp3Decoder, &mp3FrameInfo);
			frameDecoded = true;
			break;

		case ERR_MP3_INVALID_FRAMEHEADER:
			readPtr++;
			bytesLeft--;
			continue;

		case ERR_MP3_INDATA_UNDERFLOW:
			break;

		case ERR_MP3_MAINDATA_UNDERFLOW:
			/* do nothing - next call to decode will provide more mainData */
			break;

		case ERR_MP3_FREE_BITRATE_SYNC:

		default:

			break;
		}
	}

    if(FE_EOF(&currentFile))
    	return kStatus_OutOfRange;
    else
    	return s;
#endif
}

status_t MP3_ComputeSongDuration(char* path, uint32_t * seconds)
{
#if defined(_WIN64) || defined(_WIN32)

#else
	if(status!=IDLE)
		return 0;

	FIL file;
	float duration = 0;

	FRESULT result = FE_OpenFile(&file, path, FA_READ);


	assert(result == FR_OK);

	bytesLeft = 0;
	status_t s;
	uint32_t nRead;
	if(result == FR_OK)
	{

		while (FE_EOF(&file)==false)
		{
			// Condition to refill read buffer - should always be enough for a full frame
			if (bytesLeft < 2*MAINBUF_SIZE) //&& !eofReached)
			{
				// Align to 4 bytes
				//wordAlign = (4-(bytesLeft&3)) & 3;

				/* Fill read buffer */
				s = MP3_FillReadBuffer(&currentFile,readBuf, readPtr, bytesLeft, &nRead);

				if(s!=FR_OK)
					break;

				bytesLeft += nRead;
				readPtr = readBuf;
			}

			// Find start of next MP3 frame - assume EOF if no sync found
			uint32_t offset = MP3FindSyncWord(readPtr, bytesLeft);

			if (offset < 0)
			{
				readPtr = readBuf;
				bytesLeft = 0;
				continue;
			}
			else
			{
				readPtr += offset;
				bytesLeft -= offset;
			}

			switch (MP3GetNextFrameInfo(mp3Decoder, &mp3FrameInfo, readPtr))
			{
			case ERR_MP3_NONE:
//				duration += (float)mp3FrameInfo.outputSamps / (float)mp3FrameInfo.samprate;
//				readPtr += 300; // Fijarse que no se pase del arreglo
//				break;
				duration = FE_Size(&file)*8/(float)mp3FrameInfo.bitrate;
				*(seconds) = (uint32_t)duration;

				FE_CloseFile(&file);
				return kStatus_Success;

				break;

			case ERR_MP3_INVALID_FRAMEHEADER:
				readPtr++;
				bytesLeft--;
				continue;
			}

		}
	}

	return kStatus_Fail;
#endif
}

/**
 *
 */
void MP3_SetTrackChangedCB(void(*callback)(char* filename))
{
	trackChangedCB = callback;
}

/**
 *
 */
uint32_t MP3_GetSongNumber()
{
	return curSong+1;
}

/**
 *
 */
uint32_t MP3_GetQueueLength()
{
	return queueLength;
}

void MP3_SetPlaybackMode(MP3PlaybackMode mode)
{
	playbackMode = mode;
}

void MP3_GetPlaybackMode(void)
{
	return playbackMode;
}
