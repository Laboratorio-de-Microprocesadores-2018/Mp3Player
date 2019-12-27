#include <stdio.h>
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"



#include "Input.h"
#include "GUI.h"
#include "FileExplorer.h"
#include "MP3Player.h"


/* Ejemplos de callbacks que llamaria la parte grafica
 * Para cosas que necesiten que interaccionen varios modulos
 * que no se incuyen entre si.*/

//void APP_PlaySong();
//void APP_PauseSong();
//void APP_PowerOff();
//void APP_SortSongs();

void LM49450_Test()
{
    LM49450_Config config;
    LM49450_GetDefaultConfig(&config);
    config.lineInEnable = true;
    LM49450_Init(&config);
    LM49450_SetVolume(5);

    LM49450_3Dconfig * config3d;
	LM49450_GetDefault3DConfig(&config3d);
    LM49450_Set3DConfig(&config3d);

    LM49450_Enable(true);
    while(1);
}

int main(void)
{
  	/* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();


    Audio_Init();

    FE_Init();

    GUI_Init();

    MP3_Init();

    Input_Init();

	//uint32_t duration = 0;
	//MP3_ComputeSongDuration(files[2].fname,&duration);

    while(1)
    {
    	//
		FE_Tick();

		//
     	MP3_Tick();

    }


    return 0 ;
}
