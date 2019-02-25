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

#include "mp3dec.h"
#include "fsl_gpio.h"
#include "Vumeter.h"
#include "Button.h"
#include "LedMatrix.h"
#include "Input.h"
#include "ILI9341.h"

/*
 * @brief   Application entry point.
 */


int16_t  samples[NSAMPLES];
Color OSHO[]={{.val=0x262261},{.val=0x8E44AD},{.val=0xEFEFEF},{.val=0x636363},{.val=0xC1C1C1}};

Color colors[3];
int i;



void nextColor(void * b)
{
	i=(i+1)%3;
	LedMatrix_PlainColor(colors[i]);
}

void prevColor(void * b)
{
	i=(i+3-1)%3;
	LedMatrix_PlainColor(colors[i]);
}

void whiteScreen(void * b)
{
	Color c = {.val = 0x141414};
	c.R = 15;
	LedMatrix_PlainColor(c);
}
void decreaseBrightness(void * b)
{
	if(colors[i].RGB[i]>=10)
		colors[i].RGB[i]-=10;
	LedMatrix_PlainColor(colors[i]);
}
void increaseBrightness(void * b)
{
	if(colors[i].RGB[i]<250)
		colors[i].RGB[i]+=10;
	LedMatrix_PlainColor(colors[i]);
}
void clearScreen(void * b)
{
	LedMatrix_Clear();
}


int main(void)
{
  	/* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();

    //ILI9341_Init();

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


/*
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

