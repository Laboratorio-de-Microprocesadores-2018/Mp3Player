/**
 * @file Input.c
 * @brief
 *
 *
 */

#include "Input.h"
#include "Button.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_pit.h"
#include "fsl_ftm.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "clock_config.h"

/* Get source clock for PIT driver */
#define INPUT_PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
#define INPUT_PIT_HANDLER PIT0_IRQHandler
#define INPUT_PIT_IRQ_ID PIT0_IRQn
#define INPUT_PIT_CHNL kPIT_Chnl_1

#define INPUT_FTM FTM2
#define FTM_QUAD_DECODER_MODULO 20U

Button prevButton;
Button nextButton;
Button menuButton;
Button playButton;
Button selectButton;

Button * buttons[NumberOfButtons] = {&prevButton,&nextButton,&menuButton,&playButton,&selectButton};
ButtonEvent buttonsEvents[NumberOfButtons];

InputCallback inc,dec;

void Input_AttachEncoderInc(InputCallback c)
{
	inc = c;
}
void Input_AttachEncoderDec(InputCallback c)
{
	dec = c;
}
void Input_Attach(ButtonID b, ButtonEvent ev, ButtonCallback c)
{
	if(b>NumberOfButtons) return;
	Button_Attach(buttons[b],ev,c);
}

uint8_t Input_ReadNextButton(void)
{
	return GPIO_PinRead(BTN_NEXT_GPIO,BTN_NEXT_PIN);
}

uint8_t Input_ReadPrevButton(void)
{
	return GPIO_PinRead(BTN_PREV_GPIO,BTN_PREV_PIN);
}

uint8_t Input_ReadMenuButton(void)
{
	return GPIO_PinRead(BTN_MENU_GPIO,BTN_MENU_PIN);
}

uint8_t Input_ReadPlayButton(void)
{
	return GPIO_PinRead(BTN_PLAY_GPIO,BTN_PLAY_PIN);
}

uint8_t Input_ReadSelectButton(void)
{
	return GPIO_PinRead(GPIOC,BTN_SELECT_PIN); // GPIOC HARDCODEADO!
}

uint8_t Input_ReadEncoderCount(void)
{
	return (uint8_t)FTM_GetQuadDecoderCounterValue(INPUT_FTM);
}

uint8_t Input_ReadEncoderDirection(void)
{
	return (uint8_t)(FTM_GetQuadDecoderFlags(INPUT_FTM) & kFTM_QuadDecoderCountingIncreaseFlag);
}

void Input_GetEvent(ButtonID * b, ButtonEvent * ev)
{
	for(int i=0; i<NumberOfButtons; i++)
	{
		if(buttonsEvents[i] != Button_GetEvent(buttons[i]))
		{
			buttonsEvents[i] = Button_GetEvent(buttons[i]);

			(*b) = (ButtonID)buttons[i]->ID;
			(*ev)= buttonsEvents[i];
			return;
		}
	}
}

void Input_Init()
{

//    Button_Init(&prevButton, Input_ReadPrevButton, 0,PREV);
//    Button_Start(&prevButton);
//
//    Button_Init(&nextButton, Input_ReadNextButton, 0,NEXT);
//    Button_Start(&nextButton);
//
//    Button_Init(&menuButton, Input_ReadMenuButton, 0,MENU);
//    Button_Start(&menuButton);
//
//    Button_Init(&playButton, Input_ReadPlayButton, 0,PLAY);
//    Button_Start(&playButton);
//
//    Button_Init(&selectButton, Input_ReadSelectButton, 0,SELECT);
//    Button_Start(&selectButton);

    /* Initialize FTM module */
    ftm_config_t ftmConfig;
    FTM_GetDefaultConfig(&ftmConfig);
    ftmConfig.prescale = kFTM_Prescale_Divide_4;
    FTM_Init(INPUT_FTM, &ftmConfig);

    /* Set the modulo values for Quad Decoder. */
    FTM_SetQuadDecoderModuloValue(INPUT_FTM, 0U, FTM_QUAD_DECODER_MODULO);

    /* Enable the Quad Decoder mode. */
    ftm_phase_params_t phaseConfig;
    phaseConfig.enablePhaseFilter = true;
    phaseConfig.phaseFilterVal = 9U;
    phaseConfig.phasePolarity = kFTM_QuadPhaseInvert;
    FTM_SetupQuadDecode(INPUT_FTM, &phaseConfig, /* Phase A. */
                        &phaseConfig,            /* Phase B. */
                        kFTM_QuadPhaseEncode);

}

void Input_Deinit()
{
	FTM_Deinit(INPUT_FTM);
}
void Input_Start()
{
    // PIT initialization
    pit_config_t pitConfig;
    /*
	 * pitConfig.enableRunInDebug = false;
	 */
	PIT_GetDefaultConfig(&pitConfig);

	/* Init pit module */
	PIT_Init(PIT, &pitConfig);

	/* Set timer period for channel 0 */
	PIT_SetTimerPeriod(PIT, INPUT_PIT_CHNL, MSEC_TO_COUNT(BUTTON_TICK_INTERVAL_MS, INPUT_PIT_SOURCE_CLOCK));

	/* Enable timer interrupts for channel 0 */
	PIT_EnableInterrupts(PIT, INPUT_PIT_CHNL, kPIT_TimerInterruptEnable);

	/* Enable at the NVIC */
	EnableIRQ(INPUT_PIT_IRQ_ID);

	/* Start channel 0 */
	PIT_StartTimer(PIT, INPUT_PIT_CHNL);
}


void Input_Tick(void)
{
    /* Clear interrupt flag.*/
    //PIT_ClearStatusFlags(PIT, INPUT_PIT_CHNL, kPIT_TimerFlag);
    Button_Tick();

    static uint8_t lastCount;
    uint8_t currCount = FTM_GetQuadDecoderCounterValue(INPUT_FTM);
    if(currCount != lastCount)
    {
    	lastCount = currCount;

    	if(Input_ReadEncoderDirection())
    		inc(0);
    	else
    		dec(0);
    }

    //__DSB();
}
