/**
 * @file AudioI2S.c
 *
 * @brief Implement audio output using I2S protocol (SAI peripheral)
 *
 * This file implements the Audio interface using I2S digital bus and DMA.
 *
 *
 *
 *
 * Peripherals: I2S0, DMA0, DMAMUX
 *
 * Pins: C8, C1, B19, B18
 *
 * Interrupts: SAI_Callback
 *
 */

#include "Audio.h"

#if AUDIO_OUTPUT == I2S_OUTPUT

#include "fsl_sai_edma.h"
#include "fsl_dmamux.h"
#include "fsl_port.h"


#define  AUDIO_BUFFER_SIZE 2304

#undef 	SAI_XFER_QUEUE_SIZE
#define CIRC_BUFFER_LEN 2
#define SAI_XFER_QUEUE_SIZE CIRC_BUFFER_LEN

#define AUDIO_SAI 			I2S0
#define AUDIO_DMA 			DMA0
#define AUDIO_DMA_CHANNEL	0
#define AUDIO_DMA_IRQ_ID 	DMA0_IRQn

typedef struct{
	uint16_t samples[AUDIO_BUFFER_SIZE];
	uint16_t nSamples;
	uint32_t sampleRate;
	uint32_t frameNumber;
}PCM_AudioFrame;

static PCM_AudioFrame audioFrame[CIRC_BUFFER_LEN+1];
static edma_handle_t DMA_Handle;
static sai_edma_handle_t SAI_Handle;
static sai_transfer_format_t SAI_TransferFormat;

/**
 *
 */
static void SAI_Callback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData);




status_t Audio_Init()
{
	//////////////////////////////////////////////////////////
	// Sets up necessary pins.
	PORT_SetPinMux(PORTC, 8, kPORT_MuxAlt4);	/* PORTC8  is configured as I2S0_TX_MCLK */
	PORT_SetPinMux(PORTC, 1, kPORT_MuxAlt6);    /* PORTC1  is configured as I2S0_TXD0  */
	PORT_SetPinMux(PORTB, 19, kPORT_MuxAlt4);   /* PORTB19 is configured as I2S0_TX_FS */
	PORT_SetPinMux(PORTB, 18, kPORT_MuxAlt4);   /* PORTB18 is configured as I2S0_TX_BCLK*/

	//////////////////////////////////////////////////////////
	// Sets up the DMA.
	DMAMUX_Init(DMAMUX0);
	DMAMUX_SetSource(DMAMUX0, AUDIO_DMA_CHANNEL, SAI_TX_DMA_REQUEST);
	DMAMUX_EnableChannel(DMAMUX0, AUDIO_DMA_CHANNEL);

	//////////////////////////////////////////////////////////
	// Initialize EDMA.
	edma_config_t userConfig;
	EDMA_GetDefaultConfig(&userConfig);
	userConfig.enableRoundRobinArbitration = false;
	userConfig.enableHaltOnError = true;
	userConfig.enableContinuousLinkMode = false;
	userConfig.enableDebugMode = true;
	EDMA_Init(AUDIO_DMA, &userConfig);
	EDMA_CreateHandle(&DMA_Handle, AUDIO_DMA, AUDIO_DMA_CHANNEL);


	//////////////////////////////////////////////////////////
	// Initialize SAI.
	sai_config_t config;
	SAI_TxGetDefaultConfig(&config);
	SAI_TxInit(AUDIO_SAI, &config);
	SAI_TxEnable(AUDIO_SAI, true);
	SAI_TransferTxCreateHandleEDMA(AUDIO_SAI, &SAI_Handle, SAI_Callback, NULL, &DMA_Handle);

	return kStatus_Success;

}

status_t Audio_Deinit()
{

}

void Audio_Play(/*uint32_t sampleRate,*/)
{
	/* Enable Tx */
	SAI_TxEnable(AUDIO_SAI, true);

	/* Enable DMA */
	SAI_TxEnableDMA(AUDIO_SAI, kSAI_FIFORequestDMAEnable, true);

	SAI_TransferFormat.sampleRate_Hz =0000000000000000000000000000000000000000000000000;
	SAI_TransferFormat.bitWidth = 16;
	SAI_TransferFormat.stereo = kSAI_Stereo;
	//SAI_TransferFormat.masterClockHz = SAI_MCLK;
	SAI_TransferFormat.watermark = 3;


	//SAI_TransferTxSetFormatEDMA(AUDIO_SAI,&SAI_Handle,&SAI_TransferFormat,SAI_MCLK,SAI_BCLK);


}


void Audio_Pause()
{
	SAI_TransferAbortSendEDMA(AUDIO_SAI, &SAI_Handle);
}

void Audio_Resume()
{
	/* Enable Tx */
	SAI_TxEnable(AUDIO_SAI, true);

	/* Enable DMA */
	SAI_TxEnableDMA(AUDIO_SAI, kSAI_FIFORequestDMAEnable, true);


}

void Audio_Stop()
{
	SAI_TransferTerminateReceiveEDMA(AUDIO_SAI, &SAI_Handle);
	Audio_ResetQueue();
}


uint16_t * Audio_GetBackBuffer()
{
	return audioFrame[DMA_Handle.tail].samples;
}

void Audio_ResetQueue()
{
	for(int i=0; i<CIRC_BUFFER_LEN; i++)
	{
		memset(audioFrame[i].samples,0,AUDIO_BUFFER_SIZE);
		audioFrame[i].nSamples = AUDIO_BUFFER_SIZE;
		audioFrame[i].sampleRate = 44100;
	}
}


uint32_t Audio_GetCurrentFrameNumber()
{
	NVIC_DisableIRQ(AUDIO_DMA_IRQ_ID);
	uint32_t n = audioFrame[DMA_Handle.header].frameNumber;
	NVIC_EnableIRQ(AUDIO_DMA_IRQ_ID);
	return n;
}

void Audio_PushFrame(int16_t* samples, uint16_t nSamples, uint32_t sampleRate, uint32_t frameNumber)
{
	// Average stereo channels
	for(int i=0; i<nSamples; i++)
	{
		audioFrame[DMA_Handle.tail].samples[i] = ((uint16_t)(samples[i]+32768))>>4;
	}

	audioFrame[DMA_Handle.tail].nSamples = nSamples/2;
	audioFrame[DMA_Handle.tail].sampleRate = sampleRate;
	audioFrame[DMA_Handle.tail].frameNumber = frameNumber;

	sai_transfer_t transfer = {.data = (uint8_t*)audioFrame[DMA_Handle.tail].samples, .dataSize = 2*nSamples};

	status_t s = SAI_TransferSendEDMA(AUDIO_SAI,&SAI_Handle,&transfer);

	assert(s==kStatus_Success);
}

void Audio_SetSampleRate(uint32_t sr)
{
	SAI_TransferFormat.sampleRate_Hz = sr;

	SAI_TxSetFormat(AUDIO_SAI,&SAI_TransferFormat,0000,0000);

}

bool Audio_QueueIsFree()
{
	NVIC_DisableIRQ(AUDIO_DMA_IRQ_ID);
	bool b = (DMA_Handle.tail+1)%DMA_Handle.tcdSize != DMA_Handle.header;
	NVIC_EnableIRQ(AUDIO_DMA_IRQ_ID);
	return b;
}

bool Audio_QueueIsEmpty()
{
	NVIC_DisableIRQ(AUDIO_DMA_IRQ_ID);
	bool b = (DMA_Handle.tcdUsed == 0);
	NVIC_EnableIRQ(AUDIO_DMA_IRQ_ID);
	return b;
}


static void SAI_Callback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
{
	Audio_SetSampleRate(audioFrame[DMA_Handle.header].sampleRate);
}

static void EDMA_Configuration(void)
{

    edma_config_t userConfig;
    EDMA_GetDefaultConfig(&userConfig);

    userConfig.enableRoundRobinArbitration = false;
    userConfig.enableHaltOnError = true;
    userConfig.enableContinuousLinkMode = false;
    userConfig.enableDebugMode = true;

    EDMA_Init(AUDIO_DMA, &userConfig);

	/* Creates the DMA handle. */
	EDMA_CreateHandle(&DMA_Handle, AUDIO_DMA, AUDIO_DMA_CHANNEL);
}

static void DMAMUX_Configuration(void)
{
	// Sets up the DMA.
	DMAMUX_Init(DMAMUX0);

	DMAMUX_SetSource(DMAMUX0, AUDIO_DMA_CHANNEL, kDmaRequestMux0I2S0Tx);

	DMAMUX_EnableChannel(DMAMUX0, AUDIO_DMA_CHANNEL);
}


static void SAI_Configuration(void)
{
	sai_config_t config;

	SAI_TxGetDefaultConfig(&config);

	SAI_TxInit(AUDIO_SAI, &config);

	SAI_TransferTxCreateHandleEDMA(AUDIO_SAI, &SAI_Handle, SAI_Callback, NULL, &DMA_Handle);
}


#endif
