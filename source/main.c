#include <stdio.h>
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
#include "fsl_rcm.h"
#include "fsl_sim.h"
#include "Audio.h"
#include "arm_math.h"
#include "lvgl/src/lv_misc/lv_task.h"

#include "Input.h"
#include "GUI.h"
#include "FileExplorer.h"
#include "MP3Player.h"
#include "PowerManager.h"
#include "Calendar.h"
#include "fsl_edma.h"

#include "LM49450.h"

#define N 256
#define F 400
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
	GPIO_PinWrite(PWR_EN_5Vn_GPIO, PWR_EN_5Vn_PIN,0);

	GPIO_PinWrite(PWR_EN_3V3_GPIO, PWR_EN_3V3_PIN,0);

	uint32_t n = 0x0FFFF;
	while(n--)
		asm("nop");

    int16_t sinTable[N];

    for(int i=0; i<N; i++)
	{
		sinTable[i]= sin(2*PI*i/(N-1))*32768/4;
	}


    Audio_Init();

//
//    Audio_SetSampleRate(sr);
	int frameCounter = 0;

	Audio_Play();
	while(1)
	{
		if(Audio_QueueIsFree())
		{
			Audio_PushFrame(sinTable,N,1,N*F,frameCounter++);
		}
	}


}

/* */
bool powerOffReq;

void APP_Init()
{

	// Init DMA, common to many modules!
    edma_config_t userConfig;
    EDMA_GetDefaultConfig(&userConfig);
    userConfig.enableRoundRobinArbitration = false;
    userConfig.enableHaltOnError = false;
    userConfig.enableContinuousLinkMode = false;
    userConfig.enableDebugMode = true;

    EDMA_Init(DMA0, &userConfig);

	//LM49450_Test();

	if(RCM_GetPreviousResetSources(RCM) & kRCM_SourceJtag | kRCM_SourcePor)
	{
		Calendar_Init();
	}

	GPIO_PinWrite(PWR_EN_5Vn_GPIO, PWR_EN_5Vn_PIN,0);

	GPIO_PinWrite(PWR_EN_3V3_GPIO, PWR_EN_3V3_PIN,0);

	status_t s;

	s = FE_Init();
	if(s != kStatus_Success)
			while(1);


	s = MP3_Init();
	if(s != kStatus_Success)
			while(1);

	Input_Init();

	GUI_Init();
	GUI_Create();
	GUI_Task();
	GPIO_PinWrite(LCD_LED_GPIO, LCD_LED_PIN, 1U);	// Turn backlight on after creating GUI.


}

void APP_Deinit()
{
	Input_Deinit();
	GUI_Deinit();
	MP3_Deinit();
	FE_Deinit();
	EDMA_Deinit(DMA0);
}

void MP3_TaskHook(struct _lv_task_t * task)
{
	(void) task;
	MP3_Task();
}

int main(void)
{
  	/* Board hardware initialization. */
    BOARD_InitBootPins();

    PM_Recover();

    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Modules initialization */
    APP_Init();

    lv_task_create(MP3_TaskHook,5, LV_TASK_PRIO_HIGHEST, NULL);

    /* Main loop */
    while(1)
    {
    	if(GUI_PowerOffRequest())
    	{
    		SIM_SetUsbVoltRegulatorEnableMode(kSIM_UsbVoltRegEnableInAllModes);
    		MP3_Stop();
    		BOARD_DeInitPins();
    		APP_Deinit();
			PM_EnterLowPowerMode();
			PRINTF("GUI_PowerOffRequest() ERROR, shouldn't have reached here! \n");
    	}


    	//
		FE_Task();
		//
		GUI_Task();

		//
     	//MP3_Task();
    }

    return 0 ;
}
