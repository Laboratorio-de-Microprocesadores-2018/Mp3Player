/**
 * @file    main.c
 * @brief   Application entry point.
 */

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"


#include "fsl_gpio.h"

#include "Button.h"
#include "LedMatrix.h"
#include "Input.h"
#include "ILI9341.h"
#include "FileExplorer.h"
#include "MP3Player.h"

/*
 * @brief   Application entry point.
 */


uint8_t ReadSW2()
{
	return SW2_READ();
}

uint8_t ReadSW3()
{
	return SW3_READ();
}

int i;

int main(void)
{
  	/* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    BOARD_InitButtonsPins();
    BOARD_InitLEDsPins();

    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();


    FE_Init();

    MP3_Init();

   // Input_Init();

	//uint32_t duration = 0;
	//MP3_ComputeSongDuration(files[2].fname,&duration);


 	Button SW2,SW3;

	Button_Init(&SW2,ReadSW2, 0,0);
	Button_Init(&SW3,ReadSW3, 0,1);

	Button_Start(&SW2);
	Button_Start(&SW3);


    static bool prevStatus = false;
    while(1)
    {
    	bool status = FE_DriveStatus(FE_SD);
    	if(status != prevStatus)
    	{
    		prevStatus = status;
    		if(status == true)
    		{
    			PRINTF("Card inserted\n");
    			if(FE_mountDrive(FE_SD)== kStatus_Success)
    			{
    				PRINTF("Card mounted\n");

    				uint8_t k = FE_CountFilesMatching("/","*.mp3");

    				PRINTF("There are %d mp3 files in root folder\n",k);

    				MP3_Play("/",0);
    				LED_GREEN_ON();


    			}
    		}
    		else
    		{
    			PRINTF("Card removed\n");
    		}
    	}

//		ButtonEvent ev;
//		ButtonID ID;
//
//		Input_GetEvent(&ID,&ev);
//		switch(ID)
//		{
//		case NEXT:
//			break;
//		case PREV:
//			break;
//		case PLAY:
//			break;
//		case MENU:
//			break;
//		case SELECT:
//			break;
//		}

		ButtonEvent SW2Event,SW3Event;


     	MP3_Tick();


		static uint32_t count;
		count++;
		if(count == 0x4FF)
		{
			count = 0;
			Button_Tick();
		}


		if(SW2Event != Button_GetEvent(&SW2))
		{
			SW2Event = Button_GetEvent(&SW2);
			switch(SW2Event)
			{
			case PRESS_DOWN:
				MP3_PlayPause();
				LED_RED_TOGGLE();
				LED_GREEN_TOGGLE();
				break;
			case PRESS_UP:
				break;
			case LONG_PRESS_HOLD:
				LED_BLUE_TOGGLE();
			break;
			}
		}

		if(SW3Event != Button_GetEvent(&SW3))
		{
			SW3Event = Button_GetEvent(&SW3);
			switch(SW3Event)
			{
			case PRESS_DOWN:
				LED_BLUE_ON();
				break;
			case PRESS_UP:
				LED_BLUE_OFF();
				break;
			case SINGLE_CLICK:
				MP3_Next();
				break;
			case DOUBLE_CLICK:
				MP3_Prev();
			break;
			}
		}


    }


/*    //ILI9341_Init();

    LedMatrix_Init();

    Input_Init();

    colors[0].R=10;
    colors[1].G=10;
    colors[2].B=10;

    i=0;

    Input_Attach(NEXT,PRESS_UP,nextColor);
    Input_Attach(PREV,PRESS_UP,prevColor);

    Input_AttachEncoderInc(increaseBrightness);
    Input_AttachEncoderDec(decreaseBrightness);

    Input_Attach(SELECT,DOUBLE_CLICK,clearScreen);
    Input_Attach(SELECT,LONG_PRESS_START,whiteScreen);


    while(1);



    Vumeter_Init();


    for(int i=0; i<(NSAMPLES*120/128); i++)
    	samples[i]=32768;




	GPIO_PinWrite(BOARD_ADC0_SE12_GPIO, BOARD_ADC0_SE12_PIN,1);
    Vumeter_Generate(samples);
	GPIO_PinWrite(BOARD_ADC0_SE12_GPIO, BOARD_ADC0_SE12_PIN,0);

	Vumeter_Display();
*/
    return 0 ;
}


