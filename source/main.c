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
#include "PowerManager.h"

#include "LM49450.h"

/* Ejemplos de callbacks que llamaria la parte grafica
 * Para cosas que necesiten que interaccionen varios modulos
 * que no se incuyen entre si.*/

//void APP_PlaySong();
//void APP_PauseSong();
//void APP_PowerOff();
//void APP_SortSongs();
//void APP_RequestPowerOff();

void LM49450_Test()
{
    LM49450_Config config;
    LM49450_GetDefaultConfig(&config);
    config.lineInEnable = true;
    LM49450_Init(&config);
    LM49450_SetVolume(5);

    LM49450_3Dconfig config3d;
	LM49450_GetDefault3DConfig(&config3d);
    LM49450_Set3DConfig(&config3d);

    LM49450_Enable(true);
    while(1);
}

/* */
bool powerOffReq;

void APP_Init()
{
	// Power on from sleep
	if(PM_Recover())
	{

	}
	// First power on
	else
	{

	}
	FE_Init();

	GUI_Init();

	MP3_Init();

	Input_Init();
}
/*
void APP_LowPowerLoop()
{
    for(int i=0; i<NUM_TASKS; i++)
   	{
		__disable_irq();
		if(task[i].HaveToRun())
		{
			__enable_irq();
			task[i].RunPendingTasks();
		}
	}
	//APP_PrepareForSleep();
	__DSB();
	__WFI();
	//APP_RecoverFromSleep();
	__enable_irq();
	__ISB();
}
*/
int main(void)
{
  	/* Board hardware initialization. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Modules initialization */
    APP_Init();

    /* Main loop */
    while(GUI_PowerOffRequest() == false)
    {
    	//
		FE_Task();
		//
		GUI_Task();
		//
     	MP3_Task();
    }

    PM_EnterLowPowerMode();

    return 0 ;
}
