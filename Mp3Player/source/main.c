/*
 * Copyright 2016-2018 NXP Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
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
//#include "Vumeter.h"
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

    char path[256];
	uint16_t n=10;
	strcpy(path, "/");
	FILINFO files[10];

	if(FE_check4Drive()== kStatus_Success)
	{
		if(FE_mountDrive()== kStatus_Success)
		{
			FE_DirN(path,&n,files);
			PRINTF("%d",n);
		}

	}

	uint32_t duration = 0;
	//MP3_ComputeSongDuration(files[2].fname,&duration);

	MP3_Play("",0);


 	Button SW2,SW3;

	Button_Init(&SW2,ReadSW2, 0);
	Button_Init(&SW3,ReadSW3, 0);

	Button_Start(&SW2);
	Button_Start(&SW3);

	LED_GREEN_ON();

	while(1)
	{
		ButtonEvent SW2Event,SW3Event;

		static uint32_t count;
		count++;
		if(count == 0xFFF)
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
				MP3_Next();
				LED_BLUE_ON();
				break;
			case PRESS_UP:
				LED_BLUE_OFF();
				break;
			case LONG_PRESS_HOLD:
				MP3_Prev();
			break;
			}
		}

     	MP3_Tick();
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


