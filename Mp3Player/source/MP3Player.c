#include "MP3Player.h"
#include "Audio.h"
#include "FileExplorer.h"
#include "mp3dec.h"
#include "../Helix/Pub/Mp3Dec.h"

#include "math.h"

/* Player status */
typedef enum{IDLE,PLAYING,PAUSE_PENDING,PAUSE}MP3_Status;
static MP3_Status status;

/* Decoder*/
static HMP3Decoder decoder;


static int16_t SAMPLES[SAMPLES_PER_FRAME];
static char currPath[256];

status_t MP3_Init()
{
	decoder = MP3InitDecoder();

	Audio_Init();
	status = IDLE;


	for(int i=0; i<SAMPLES_PER_FRAME; i++)
	{
		//SAMPLES[i] = (int16_t)(((uint16_t)(((float)i/(float)SAMPLES_PER_FRAME)*65536))-32768);
		SAMPLES[i] = sin( (float)i/SAMPLES_PER_FRAME * 2 * 3.14)*32767;

	}

		//SAMPLES[i] = (int16_t)(((uint16_t)(((float)i/(float)SAMPLES_PER_FRAME)*65536))-32768);


}


void MP3_Play(char * path)
{
	Audio_SetSampleRate(44100);
	FIL currentFile;

	FRESULT result = FE_OpenFile(&currentFile, path, FA_OPEN_EXISTING );

	if(result == FR_OK)
	{
		int buffer[1024];
		int bytesRead;


		while(1)
		{
			FE_ReadFile(&currentFile,(void*)buffer,1024,&bytesRead);
			int offset = MP3FindSyncWord(buffer,1024);

			if(offset != -1)
				break;
		}







		Audio_FillBackBuffer(SAMPLES);
		Audio_Play();
		status = PLAYING;

	}

	// Abrir el archivo
	// Sincronizar con helix
	// Decodificar el primer frame
	// Pasar al status PLAYING



}



void MP3_Tick()
{

	switch(status)
	{
	case IDLE:

		break;

	case PLAYING:
		while(Audio_BackBufferIsFree())
		{
			Audio_FillBackBuffer(SAMPLES);

		}
		break;

	case PAUSE_PENDING:

		break;

	case PAUSE:

		break;
	}
}

