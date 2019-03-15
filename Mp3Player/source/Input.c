#include "Input.h"
#include "Button.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_pit.h"
#include "fsl_ftm.h"
#include "fsl_common.h"
#include "clock_config.h"

/* Get source clock for PIT driver */
#define INPUT_PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
#define INPUT_PIT_HANDLER PIT0_IRQHandler
#define INPUT_PIT_IRQ_ID PIT0_IRQn
#define INPUT_PIT_CHNL kPIT_Chnl_1

#define INPUT_FTM FTM2
#define FTM_QUAD_DECODER_MODULO 20U

#define MENU_GPIO	GPIOC
#define MENU_PORT	PORTC
#define MENU_PIN	7U

#define PREV_GPIO	GPIOC
#define PREV_PORT	PORTC
#define PREV_PIN	0U

#define PLAY_GPIO	GPIOC
#define PLAY_PORT	PORTC
#define PLAY_PIN	9U

#define NEXT_GPIO	GPIOC
#define NEXT_PORT	PORTC
#define NEXT_PIN	8U

#define SELECT_GPIO	GPIOC
#define SELECT_PORT	PORTC
#define SELECT_PIN	1U

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

uint8_t Input_ReadNextButton()
{
	return GPIO_PinRead(NEXT_GPIO,NEXT_PIN);
}

uint8_t Input_ReadPrevButton()
{
	return GPIO_PinRead(PREV_GPIO,PREV_PIN);
}

uint8_t Input_ReadMenuButton()
{
	return GPIO_PinRead(MENU_GPIO,MENU_PIN);
}

uint8_t Input_ReadPlayButton()
{
	return GPIO_PinRead(NEXT_GPIO,PLAY_PIN);
}

uint8_t Input_ReadSelectButton()
{
	return GPIO_PinRead(SELECT_GPIO,SELECT_PIN);
}

uint8_t Input_ReadEncoderCount()
{
	FTM_GetQuadDecoderCounterValue(INPUT_FTM);
}

uint8_t Input_ReadEncoderDirection()
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
    /* Ports Clock Gate Control: Clock enabled */
    //CLOCK_EnableClock(kCLOCK_PortB);
    CLOCK_EnableClock(kCLOCK_PortC);

    gpio_pin_config_t GPIOConfig =
    {
        .pinDirection = kGPIO_DigitalInput,
        .outputLogic = 0U
    };

    // Input pin PORT configuration
    port_pin_config_t portConfig = {/* Internal pull-up/down resistor is disabled */
    							   kPORT_PullUp,
                                   /* Fast slew rate is configured */
                                   kPORT_FastSlewRate,
                                   /* Passive filter is disabled */
                                   kPORT_PassiveFilterDisable,
                                   /* Open drain is disabled */
                                   kPORT_OpenDrainDisable,
                                   /* Low drive strength is configured */
                                   kPORT_LowDriveStrength,
                                   /* Pin is configured as PTA4 */
                                   kPORT_MuxAsGpio,
                                   /* Pin Control Register fields [15:0] are not locked */
                                   kPORT_UnlockRegister};


    PORT_SetPinConfig(MENU_PORT, MENU_PIN, &portConfig);
    GPIO_PinInit(MENU_GPIO, MENU_PIN, &GPIOConfig);

    PORT_SetPinConfig(PREV_PORT, PREV_PIN, &portConfig);
    GPIO_PinInit(PREV_GPIO, PREV_PIN, &GPIOConfig);

    PORT_SetPinConfig(PLAY_PORT, PLAY_PIN, &portConfig);
    GPIO_PinInit(PLAY_GPIO, PLAY_PIN, &GPIOConfig);

    PORT_SetPinConfig(NEXT_PORT, NEXT_PIN, &portConfig);
    GPIO_PinInit(NEXT_GPIO, NEXT_PIN, &GPIOConfig);

    PORT_SetPinConfig(SELECT_PORT, SELECT_PIN, &portConfig);
    GPIO_PinInit(SELECT_GPIO, SELECT_PIN, &GPIOConfig);

    portConfig.mux = kPORT_MuxAlt6;
    portConfig.pullSelect = kPORT_PullUp;
    PORT_SetPinConfig(PORTB, 18, &portConfig);
    PORT_SetPinConfig(PORTB, 19, &portConfig);

    Button_Init(&prevButton, Input_ReadPrevButton, 0,PREV);
    Button_Start(&prevButton);

    Button_Init(&nextButton, Input_ReadNextButton, 0,NEXT);
    Button_Start(&nextButton);

    Button_Init(&menuButton, Input_ReadMenuButton, 0,MENU);
    Button_Start(&menuButton);

    Button_Init(&playButton, Input_ReadPlayButton, 0,PLAY);
    Button_Start(&playButton);

    Button_Init(&selectButton, Input_ReadSelectButton, 0,SELECT);
    Button_Start(&selectButton);

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

void INPUT_PIT_HANDLER(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, INPUT_PIT_CHNL, kPIT_TimerFlag);
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

    __DSB();
}
