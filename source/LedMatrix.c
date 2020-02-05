/**
 * @file LedMatrix.c
 * @brief
 *
 *
 */

#include "LedMatrix.h"

#include "Assert.h"
#include "string.h"
#include "Board.h"
#include "Font.h"
#include "limits.h"

#include "fsl_dspi.h"
#include "fsl_dspi_edma.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"

/////////////////////////////////////////////////////////////////////////////////
//                       Constants and macro definitions                       //
/////////////////////////////////////////////////////////////////////////////////

#define TEXT_BUFFER_LEN 		40
#define NUMBER_OF_LEDS 			(MATRIX_WIDTH * MATRIX_HEIGHT)
#define SCREEN_BUFFER_SIZE  	(NUMBER_OF_LEDS * 3)
#define SCROLL_PERIOD 			0.17

#define LED_SPI 				SPI2
#define LED_DMA_MUX 			DMAMUX
#define LED_DMA 				DMA0
#define LED_DMA_SOURCE 			kDmaRequestMux0SPI2


#define DSPI_MASTER_CLK_FREQ	CLOCK_GetFreq(DSPI0_CLK_SRC)
#define LED_PCS_FOR_INIT 		kDSPI_Pcs0
#define LED_PCS_FOR_TRANSFER 	kDSPI_MasterPcs0
#define PADDING_SIZE 			13
#define TRANSFER_SIZE 			384U         /* Transfer dataSize */
#define TRANSFER_BAUDRATE 		2500000U /* Transfer baudrate 2.5Mhz */
#define TX_CHANNEL 				1
#define RX_CHANNEL 				2
#define INTERMEDIARY_CHANNEL 	3

#define COLOR_TEST 				0

/////////////////////////////////////////////////////////////////////////////////
//                    Enumerations, structures and typedefs                    //
/////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* DSPI user callback */
static void DSPI_MasterUserCallback(SPI_Type *base, dspi_master_edma_handle_t *handle, status_t status, void *userData);


/////////////////////////////////////////////////////////////////////////////////
//                   Local variable definitions ('static')                     //
/////////////////////////////////////////////////////////////////////////////////

// Back buffer of screen
static Color screenBuffer[NUMBER_OF_LEDS];

// Text buffer
static char textBuffer[TEXT_BUFFER_LEN+1];
// Length of text
static uint16_t textLength;

// Rendered text buffer
static Color screenTextBuffer[MATRIX_HEIGHT][(FONT_CHAR_WIDTH + FONT_SEPARATION)*(TEXT_BUFFER_LEN+1)];
// Rendered text buffer length
static uint16_t screenTextBufferLength;

// Bit to be send to SPI

static uint16_t bitstream[2*PADDING_SIZE+TRANSFER_SIZE] = {};

static bool init = false;

// Table to encode the colors through SPI
static const uint16_t bitCode[] = {
    0b100100100100,
    0b100100100110,
    0b100100110100,
    0b100100110110,
    0b100110100100,
    0b100110100110,
    0b100110110100,
    0b100110110110,
    0b110100100100,
    0b110100100110,
    0b110100110100,
    0b110100110110,
    0b110110100100,
    0b110110100110,
    0b110110110100,
    0b110110110110
};

static dspi_master_edma_handle_t SPI_handle;
static edma_handle_t dspiEdmaMasterRxRegToRxDataHandle;
static edma_handle_t dspiEdmabitstreamToIntermediaryHandle;
static edma_handle_t dspiEdmaMasterIntermediaryToTxRegHandle;
static dspi_transfer_t transfer;
static volatile bool isTransferCompleted = false;


// Scroll mode: once or continuous
static ScrollMode scrollMode;
// Flag of scrolling text
static bool scrolling;
// Index to rendered buffer
static uint16_t scrollIndex;

/////////////////////////////////////////////////////////////////////////////////
//                          Local functions declarations                       //
/////////////////////////////////////////////////////////////////////////////////

// Send update command
static void LedMatrix_Update();
// Periodic interrupt to scroll text
static void LedMatrix_ScrollText();

/////////////////////////////////////////////////////////////////////////////////
//                          Global Function Definitions                        //
/////////////////////////////////////////////////////////////////////////////////

void LedMatrix_Init(void)
{
	/* DMA MUX init */
	DMAMUX_Init(LED_DMA_MUX);
	//DMAMUX_SetSource(LED_DMA_MUX, TX_CHANNEL,(uint8_t)LED_DMA_Tx_SOURCE);
	//DMAMUX_EnableChannel(LED_DMA_MUX, TX_CHANNEL);
	DMAMUX_SetSource(LED_DMA_MUX, RX_CHANNEL,(uint8_t)LED_DMA_SOURCE);
	DMAMUX_EnableChannel(LED_DMA_MUX, RX_CHANNEL);

	/* SPI master config */
	dspi_master_config_t masterConfig;
	masterConfig.whichCtar = kDSPI_Ctar0;
	masterConfig.ctarConfig.baudRate = TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.bitsPerFrame = 12;
	masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveHigh;
	masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseFirstEdge;
	masterConfig.ctarConfig.direction = kDSPI_MsbFirst;
	masterConfig.ctarConfig.pcsToSckDelayInNanoSec = 0;
	masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec = 0;
	masterConfig.ctarConfig.betweenTransferDelayInNanoSec =0;
	masterConfig.whichPcs = LED_PCS_FOR_INIT;
	masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;
	masterConfig.enableContinuousSCK = false;
	masterConfig.enableRxFifoOverWrite = false;
	masterConfig.enableModifiedTimingFormat = false;
	masterConfig.samplePoint = kDSPI_SckToSin0Clock;

	uint32_t srcClock_Hz = DSPI_MASTER_CLK_FREQ;
	DSPI_MasterInit(LED_SPI, &masterConfig, srcClock_Hz);

	/* Set up dspi master */

	EDMA_CreateHandle(&(dspiEdmaMasterRxRegToRxDataHandle), LED_DMA, RX_CHANNEL);

	EDMA_CreateHandle(&(dspiEdmabitstreamToIntermediaryHandle), LED_DMA, INTERMEDIARY_CHANNEL);

	EDMA_CreateHandle(&(dspiEdmaMasterIntermediaryToTxRegHandle), LED_DMA, TX_CHANNEL);

	DSPI_MasterTransferCreateHandleEDMA(LED_SPI, &SPI_handle, DSPI_MasterUserCallback,
										NULL, &dspiEdmaMasterRxRegToRxDataHandle,
										&dspiEdmabitstreamToIntermediaryHandle,
										&dspiEdmaMasterIntermediaryToTxRegHandle);

	/* Config transfer */
	transfer.txData = (uint8_t*)bitstream;
	transfer.rxData = NULL;
	transfer.dataSize = (2*PADDING_SIZE+TRANSFER_SIZE)*2;
	transfer.configFlags = kDSPI_MasterCtar0 | LED_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;

	memset(&bitstream[0],0,PADDING_SIZE*sizeof(uint16_t));
	memset(&bitstream[PADDING_SIZE+TRANSFER_SIZE],0,PADDING_SIZE*sizeof(uint16_t));

	scrollIndex = 0;
	scrolling=false;

	init = true;

	//sysTickInit();
	// Add to systick periodic callback to scroll text
	//sysTickAddCallback(LedMatrix_ScrollText,SCROLL_PERIOD);

	// Startup test
	LedMatrix_ShortColorTest();

	LedMatrix_Clear();
}

void LedMatrix_Deinit()
{
	DMAMUX_Deinit(DMAMUX);
	DSPI_Deinit(LED_SPI);
}
void LedMatrix_Clear(void)
{
	Color c = {.val=0};
	LedMatrix_PlainColor(c);
}


void LedMatrix_PrintLed(uint8_t row, uint8_t col, Color c)
{
	uint8_t index = row*MATRIX_WIDTH + col;
	screenBuffer[index] = c;

	bitstream[PADDING_SIZE+row*MATRIX_WIDTH+col+0] = bitCode[c.G >> 4 ];
	bitstream[PADDING_SIZE+row*MATRIX_WIDTH+col+1] = bitCode[c.G & 0xF];
	bitstream[PADDING_SIZE+row*MATRIX_WIDTH+col+2] = bitCode[c.R >> 4 ];
	bitstream[PADDING_SIZE+row*MATRIX_WIDTH+col+3] = bitCode[c.R & 0xF];
	bitstream[PADDING_SIZE+row*MATRIX_WIDTH+col+4] = bitCode[c.B >> 4 ];
	bitstream[PADDING_SIZE+row*MATRIX_WIDTH+col+5] = bitCode[c.B & 0xF];

	LedMatrix_Update();
}

//~190us
void LedMatrix_PrintScreen(Color * screen)
{
	for(int i=0; i<MATRIX_HEIGHT; i++)
	{
		for(int j=0; j<MATRIX_WIDTH; j++)
		{
			bitstream[PADDING_SIZE+(MATRIX_WIDTH*i+j)*6+0] = bitCode[screen[i*MATRIX_WIDTH+j].G >> 4 ];
			bitstream[PADDING_SIZE+(MATRIX_WIDTH*i+j)*6+1] = bitCode[screen[i*MATRIX_WIDTH+j].G & 0xF];
			bitstream[PADDING_SIZE+(MATRIX_WIDTH*i+j)*6+2] = bitCode[screen[i*MATRIX_WIDTH+j].R >> 4 ];
			bitstream[PADDING_SIZE+(MATRIX_WIDTH*i+j)*6+3] = bitCode[screen[i*MATRIX_WIDTH+j].R & 0xF];
			bitstream[PADDING_SIZE+(MATRIX_WIDTH*i+j)*6+4] = bitCode[screen[i*MATRIX_WIDTH+j].B >> 4 ];
			bitstream[PADDING_SIZE+(MATRIX_WIDTH*i+j)*6+5] = bitCode[screen[i*MATRIX_WIDTH+j].B & 0xF];

			screenBuffer[i*MATRIX_WIDTH+j] = screen[i*MATRIX_WIDTH+j];
		}
	}
	LedMatrix_Update();
}

void LedMatrix_PlainColor(Color c)
{
	for(int i=0; i<NUMBER_OF_LEDS; i++)
	{
		screenBuffer[i]   = c;
		bitstream[PADDING_SIZE+i*6+0] = bitCode[c.G >> 4 ];
		bitstream[PADDING_SIZE+i*6+1] = bitCode[c.G & 0xF];
		bitstream[PADDING_SIZE+i*6+2] = bitCode[c.R >> 4 ];
		bitstream[PADDING_SIZE+i*6+3] = bitCode[c.R & 0xF];
		bitstream[PADDING_SIZE+i*6+4] = bitCode[c.B >> 4 ];
		bitstream[PADDING_SIZE+i*6+5] = bitCode[c.B & 0xF];
	}

	LedMatrix_Update();
}

void LedMatrix_ShortColorTest(void)
{
#if (COLOR_TEST==1)
	uint64_t counter;

	//  Colores estaticos
	static Color R = {.RGB={10,0,0}};
	static Color G = {.RGB={0,10,0}};
	static Color B = {.RGB={0,0,10}};
	static Color W = {.RGB={10,10,10}};

	LedMatrix_PlainColor(G);
	counter = 0x5FFFFF;
	while(counter--);

	LedMatrix_PlainColor(R);
	counter = 0x5FFFFF;
	while(counter--);

	LedMatrix_PlainColor(B);
	counter = 0x5FFFFF;
	while(counter--);

	LedMatrix_PlainColor(W);
	counter = 0x5FFFFF;
	while(counter--);

	Color screen1[8][8] ={	{R,R,G,G,B,B,W,W},
							{R,R,G,G,B,B,W,W},
							{R,R,G,G,B,B,W,W},
							{R,R,G,G,B,B,W,W},
							{R,R,G,G,B,B,W,W},
							{R,R,G,G,B,B,W,W},
							{R,R,G,G,B,B,W,W},
							{R,R,G,G,B,B,W,W}};
	LedMatrix_PrintScreen(&screen1[0][0]);
	counter = 0x5FFFFF;
	while(counter--);

	Color screen2[8][8] ={	{R,R,R,R,R,R,R,R},
							{R,R,R,R,R,R,R,R},
							{G,G,G,G,G,G,G,G},
							{G,G,G,G,G,G,G,G},
							{B,B,B,B,B,B,B,B},
							{B,B,B,B,B,B,B,B},
							{W,W,W,W,W,W,W,W},
							{W,W,W,W,W,W,W,W}};
	LedMatrix_PrintScreen(&screen2[0][0]);
	counter = 0x5FFFFF;
	while(counter--);

	LedMatrix_Clear();
	counter = 0xFFF;
	while(counter--);
#endif

}
void LedMatrix_ColorTest(void)
{
	uint64_t counter = 0xFFFFFF;

	////////////////////////////////////////////////////////////////////////////
	//  Colores estaticos
	static Color R = {.RGB={10,0,0}};
	static Color G = {.RGB={0,10,0}};
	static Color B = {.RGB={0,0,10}};

	static Color W = {.RGB={6,6,6}};

	////////////////////////////////////////////////////////////////////////////
	// Prueba led por led
	Color colors[3] = {R,G,B};
	LedMatrix_Clear();

	counter = 0xFFFFF;
	while(counter--);
/*
	for(int i=0; i<MATRIX_HEIGHT; i++)
	{
		for(int j=0; j<MATRIX_WIDTH; j++)
		{
			LedMatrix_PrintLed(i,j,colors[i%3]);

			counter = 0xFFFFF;
			while(counter--);
		}
	}
	*/
	////////////////////////////////////////////////////////////////////////////
	// Prueba de colores y brillo creciente

	for(int i=0; i<10; i++)
	{
		LedMatrix_PlainColor(R);
		counter = 0xFFFFF;
		while(counter--);

		LedMatrix_PlainColor(G);
		counter = 0xFFFFF;
		while(counter--);

		LedMatrix_PlainColor(B);
		counter = 0xFFFFF;
		while(counter--);

		R.R +=10;
		G.G +=10;
		B.B +=10;
	}
	LedMatrix_PlainColor(W);
	counter = 0xFFFFFF;
	while(counter--);

	/*
	////////////////////////////////////////////////////////////////////////////
	// Prueba de texto
	Color fontColor = {0,0,10};
	Color backgroundColor = {10,10,10};
	LedMatrix_Print("ITBA ", 5,fontColor,backgroundColor, SCROLL_CONTINUOUS);
	delayMs(20000);
	LedMatrix_StopScrolling();

	////////////////////////////////////////////////////////////////////////////

	// Prueba de texto
	LedMatrix_Mirror(true);
	LedMatrix_Print("ITBA ", 5,fontColor,backgroundColor, SCROLL_CONTINUOUS);
	delayMs(20000);
	LedMatrix_StopScrolling();
	LedMatrix_Mirror(false);
		////////////////////////////////////////////////////////////////////////////
	*/
	R.R = 20;
	G.G = 20;
	B.B = 20;

	Color screen1[8][8] ={	{R,R,G,G,B,B,W,W},
							{R,R,G,G,B,B,W,W},
							{R,R,G,G,B,B,W,W},
							{R,R,G,G,B,B,W,W},
							{R,R,G,G,B,B,W,W},
							{R,R,G,G,B,B,W,W},
							{R,R,G,G,B,B,W,W},
							{R,R,G,G,B,B,W,W}};
	LedMatrix_PrintScreen(screen1[0]);
	counter = 0xFFFFFF;
	while(counter--);

	Color screen2[8][8] ={	{R,R,R,R,R,R,R,R},
							{R,R,R,R,R,R,R,R},
							{G,G,G,G,G,G,G,G},
							{G,G,G,G,G,G,G,G},
							{B,B,B,B,B,B,B,B},
							{B,B,B,B,B,B,B,B},
							{W,W,W,W,W,W,W,W},
							{W,W,W,W,W,W,W,W}};

	LedMatrix_PrintScreen(screen2[0]);
	counter = 0xFFFFFF;
	while(counter--);
}


void LedMatrix_Print(char * c, uint8_t len,Color fontColor, Color backgroundColor, ScrollMode mode)
{
	//ASSERT(len < TEXT_BUFFER_LEN);

	if(mode == SCROLL_ONCE)
	{
		memcpy(textBuffer+1,c,len);

		len++;
		textBuffer[len] = ' ';
		len++;
		textBuffer[0] = ' ';
	}
	else
		memcpy(textBuffer,c,len);

	textLength = len;
	screenTextBufferLength = textLength*(FONT_CHAR_WIDTH+FONT_SEPARATION);
	scrollMode = mode;
	scrollIndex = 0;
	// Build  screen memory
	for(int i=0; i<len; i++)
	{
		for(int row=0; row<MATRIX_WIDTH; row++)
		{
			for(int col=0; col< FONT_CHAR_WIDTH; col++)
			{
				if(FONT_BIT(textBuffer[i], col, row) == true)
					screenTextBuffer[row][i*(FONT_CHAR_WIDTH + FONT_SEPARATION)+col]=fontColor;
				else
					screenTextBuffer[row][i*(FONT_CHAR_WIDTH + FONT_SEPARATION)+col]=backgroundColor;
			}

			for(int col= FONT_CHAR_WIDTH; col<FONT_CHAR_WIDTH + FONT_SEPARATION; col++)
				screenTextBuffer[row][i*(FONT_CHAR_WIDTH + FONT_SEPARATION)+col] = backgroundColor;
		}
	}
	scrolling=true;
}

static void LedMatrix_ScrollText()
{
	if(scrolling)
	{
		for(int row=0; row<MATRIX_HEIGHT; row++)
		{
			for(int col=0; col<MATRIX_WIDTH; col++)
			{
				screenBuffer[col+row*MATRIX_WIDTH] = screenTextBuffer[row][(scrollIndex+col)%screenTextBufferLength];
			}
		}
		LedMatrix_Update();

		scrollIndex = scrollIndex + 1;

		if(scrollMode == SCROLL_ONCE)
		{
			if(scrollIndex == screenTextBufferLength-8)
			{
				scrolling = false;
				scrollIndex = 0;
			}
		}
		else if(scrollMode == SCROLL_CONTINUOUS)
			scrollIndex %= screenTextBufferLength;
	}
}

void LedMatrix_StopScrolling()
{
	scrolling=false;
}

void LedMatrix_StartScrolling()
{
	scrolling=true;
}

static void LedMatrix_Update()
{
	if(init == true)
		DSPI_MasterTransferEDMA(LED_SPI, &SPI_handle, &transfer);
}

static void DSPI_MasterUserCallback(SPI_Type *base, dspi_master_edma_handle_t *handle, status_t status, void *userData)
{

    isTransferCompleted = true;
}
