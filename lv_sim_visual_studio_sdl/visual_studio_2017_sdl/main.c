/**
* @file main
*
*/

/*********************
*      INCLUDES
*********************/
#include <stdlib.h>
#include <Windows.h>
#include <SDL.h>
#include <SDL_mutex.h>

#include "../../Mp3Player/source/GUI.h"
#include "lv_examples/lv_apps/demo/demo.h"

/*********************
*      DEFINES
*********************/

/**********************
*      TYPEDEFS
**********************/

/**********************
*  STATIC PROTOTYPES
**********************/

static int tick_thread(void *data);
void mp3Player_thread(void* data);
/**********************
*  STATIC VARIABLES
**********************/


/**********************
*      MACROS
**********************/

/**********************
*   GLOBAL FUNCTIONS
**********************/

int main(int argc, char** argv)
{
    /*Initialize GUI*/
	GUI_Init();
	GUI_Create();
	
	//demo_create();
	/* Tick init.
	* You have to call 'lv_tick_inc()' in every milliseconds
	* Create an SDL thread to do this*/
	SDL_CreateThread(tick_thread, "tick", NULL);
  
	/* Thread to simulate MP3 module*/
	SDL_CreateThread(mp3Player_thread, "mp3Player", NULL);


    while (1) {
        /* Periodically call the lv_task handler.
        * It could be done in a timer interrupt or an OS task too.*/
		GUI_Task();
        Sleep(10);       /*Just to let the system breathe */
    }

    return 0;
}

/**********************
*   STATIC FUNCTIONS
**********************/
typedef enum {
	IDLE,
	PLAYING,
	PAUSE
}status_t;

int elapsed = 0;
int duration = 145;
status_t status = IDLE;

//
//void MP3_Play(char* path)
//{
//	elapsed = 0;
//	status = PLAYING;
//}
//
//void MP3_Pause()
//{
//	status = PAUSE;
//}
//
//void MP3_GetStatus()
//{
//	return status;
//}
//int MP3_GetTrackProgress()
//{
//	return elapsed*100/duration;
//}
//
//int MP3_GetPlaytime()
//{
//	return elapsed;
//}
//
//int MP3_GetDuration()
//{
//	return duration;
//}

void mp3Player_thread(void* data)
{
	SDL_mutex* mutex = SDL_CreateMutex();

	while (1)
	{
		switch (status)
		{
		case IDLE:
			break;
		case PLAYING:
			if (elapsed < duration)
				elapsed++;
			else
				status = IDLE;
			break;
		case PAUSE:
			break;
		}
		SDL_Delay(1000);
	}
}
/**
* A task to measure the elapsed time for LittlevGL
* @param data unused
* @return never return
*/
static int tick_thread(void *data)
{
    while (1) {
        lv_tick_inc(5);
        SDL_Delay(5);   /*Sleep for 1 millisecond*/
    }

    return 0;
}
