#include "MP3Player.h"
#include "Audio.h"
#include "FileExplorer.h"
#include "mp3dec.h"
#include "Vumeter.h"

#include "math.h"
#include "assert.h"

#include "fsl_debug_console.h"

#define  READ_BUFFER_SIZE  (1024*8)
/* Every 3 frames the vumeter is updated */
#define VUMETER_UPDATE_MODULO 3

/* Player status */
typedef enum{IDLE,PLAYING,PLAYING_LAST_FRAMES,PAUSE_PENDING,PAUSE}MP3_Status;
static MP3_Status status;

/* Decoder*/
static HMP3Decoder mp3Decoder;

static MP3FrameInfo mp3FrameInfo;

/* Current file */
static FIL currentFile;
static FILINFO currentFileInfo;
//static DIR currentDir;
static char curPath[256];
static FE_FILE_SORT_TYPE plSortCriteria = ABC;	// Playlist sort criteria
static uint8_t fileIndexLut[MAX_FILES_PER_DIR];
static uint8_t fileIndexLutSize;
static uint8_t curSong;


static uint8_t readBuf[READ_BUFFER_SIZE];
static uint8_t * readPtr;
static uint32_t bytesLeft;

static int16_t audioBuf[MAX_SAMPLES_PER_FRAME];

static uint32_t frameCounter;
static float playbackTime;

static status_t MP3_FillReadBuffer(FIL * fp, uint8_t *readBuf, uint8_t *readPtr, uint16_t bytesLeft, uint32_t * nRead);
static status_t MP3_DecodeFrame();
static void MP3_PlayCurrentSong();

status_t MP3_Init()
{
	mp3Decoder = MP3InitDecoder();

	if(mp3Decoder == 0)
	{
		// This means the memory allocation failed. This typically happens if there is not
		// enough heap memory.
		assert(mp3Decoder);
		return kStatus_Fail;
	}

	Audio_Init();

	Vumeter_Init();

	status = IDLE;

	return kStatus_Success;
}


void MP3_Play(char * dirPath, uint8_t index)
{
	if(status == PLAYING)
		MP3_Stop();


	strcpy(curPath,dirPath);

	// Sort files
	fileIndexLutSize = FE_Sort(plSortCriteria, dirPath, ".mp3", fileIndexLut);
	uint8_t i = 0;
	while(fileIndexLut[i]!=index && i<fileIndexLutSize) i++;
	if(fileIndexLut[i]==index)
		curSong = i;


	// TEMPORAL
//	curSong = index;
//	fileIndexLut[curSong] = index;


	MP3_PlayCurrentSong();
}

static void MP3_PlayCurrentSong()
{
	// Open file
	FRESULT result = FE_OpenFileN(  curPath,
									&currentFile,
									&currentFileInfo,
									FA_READ,
									fileIndexLut[curSong],
									"*.mp3");

	if(result == FR_OK)
	{
		PRINTF("Playing '%s' \n",currentFileInfo.fname);

		memset(audioBuf,0,MAX_SAMPLES_PER_FRAME);

		Audio_FillBackBuffer(audioBuf,MAX_SAMPLES_PER_FRAME,44100,0);
		Audio_FillBackBuffer(audioBuf,MAX_SAMPLES_PER_FRAME,44100,0);
		Audio_FillBackBuffer(audioBuf,MAX_SAMPLES_PER_FRAME,44100,0);

		Audio_Play();

		readPtr = readBuf;
		bytesLeft = 0;
		frameCounter = 0;
		playbackTime = 0;

		status = PLAYING;
	}

}

void MP3_Stop()
{
	Audio_Stop();
	Vumeter_Clear();
	FE_CloseFile(&currentFile);
	status = IDLE;
}

void MP3_Next()
{
	if(status != IDLE)
		MP3_Stop();

	/* DESCOMENTAR CUANDO ESTE LO DE GENERAR LA PLAYLIST
	// Move to next song in current folder
	curSong++;

	// Wrap around if reached the end
	if(fileIndexLut[curSong]==EOF)
		curSong = 0;
	*/
	fileIndexLut[curSong]++;

	MP3_PlayCurrentSong();

}

void MP3_Prev()
{
	if(status != IDLE)
		MP3_Stop();

	if(playbackTime <5 && fileIndexLut[curSong] > 0)
		fileIndexLut[curSong]--;

	MP3_PlayCurrentSong();
}

void MP3_PlayPause()
{
	if(status==PLAYING)
	{
		Audio_Pause();
		status = PAUSE;
		PRINTF("Paused\n");
	}
	else if(status == PAUSE)
	{
		Audio_Resume();
		status = PLAYING;
		PRINTF("Playing '%s' \n",currentFileInfo.fname);

	}
}

void MP3_Tick()
{

	switch(status)
	{
	case IDLE:

		break;

	case PLAYING:

		// Decode as many frames as possible
		while(Audio_BackBufferIsFree())
		{
			status_t s = MP3_DecodeFrame();
			if(s==kStatus_Success)
			{
				if(Vumeter_BackBufferEmpty()  &&  frameCounter%VUMETER_UPDATE_MODULO == 0)
					Vumeter_Generate(audioBuf);

				Audio_FillBackBuffer(audioBuf,
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
		if(Audio_BackBufferIsEmpty())
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
}
/**
 *    @brief
 */
static status_t MP3_FillReadBuffer(FIL * fp, uint8_t *readBuf, uint8_t *readPtr, uint16_t bytesLeft, uint32_t * nRead)
{
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


}

static status_t MP3_DecodeFrame()
{
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


}

status_t MP3_ComputeSongDuration(char* path, uint32_t * seconds)
{
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
}
