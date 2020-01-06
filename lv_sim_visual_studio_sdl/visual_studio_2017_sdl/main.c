
/*********************
*      INCLUDES
*********************/
//#include <stdlib.h>

#include <SDL.h>
#include <SDL_mutex.h>

#include "GUI.h"
#include "FileExplorer.h"
#include "MP3Player.h"

int main(int argc, char** argv)
{
	GUI_Init();
	GUI_Create();
	FE_Init();
	MP3_Init();

    while (GUI_PowerOffRequest()==false)
	{
		GUI_Task();
		MP3_Task();
        SDL_Delay(5);       /*Just to let the system breathe */
    }

    return 0;
}


