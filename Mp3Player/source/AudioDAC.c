#include "Audio.h"

#if AUDIO_OUTPUT == DAC

#include "fsl_dac.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"
#include "fsl_pit.h"

#define  AUDIO_BUFFER_SIZE 2304
#define  CIRC_BUFFER_LEN 2


#define AUDIO_DAC DAC0
#define AUDIO_DMA_CHANNEL 0U
#define AUDIO_DMA_DAC_SOURCE 45U
#define AUDIO_DMA_BASEADDR DMA0
#define DAC_DATA_REG_ADDR 0x400cc000U //DAC_GetBufferAddress(AUDIO_DAC)
#define AUDIO_DMA_IRQ_ID DMA0_IRQn


#define AUDIO_DMA_ALWAYS_ENABLE_SOURCE 58U
#define AUDIO_PIT PIT
#define AUDIO_PIT_CHNL  kPIT_Chnl_0 // AUDIO_DMA_CHANNEL tiene que ser igual al numero de PIT!!


typedef struct{
	uint16_t samples[AUDIO_BUFFER_SIZE];
	uint16_t nSamples;
	uint32_t sampleRate;
	uint32_t frameNumber;
}PCM_AudioFrame;

static PCM_AudioFrame audioFrame[CIRC_BUFFER_LEN+1];

#define TCD_QUEUE_SIZE CIRC_BUFFER_LEN
AT_NONCACHEABLE_SECTION_ALIGN(static edma_tcd_t tcdMemoryPoolPtr[TCD_QUEUE_SIZE + 1], sizeof(edma_tcd_t));


/*
static volatile uint8_t circBufferHead;
static volatile uint8_t circBufferTail;

// Circular buffer
#define BUFFER_POP  circBufferTail = (circBufferTail+1)%CIRC_BUFFER_LEN
#define BUFFER_PUSH circBufferHead = (circBufferHead+1)%CIRC_BUFFER_LEN
#define BUFFER_IS_EMPTY (circBufferHead == circBufferTail)
#define BUFFER_IS_FULL (((circBufferHead+1)%CIRC_BUFFER_LEN)==circBufferTail)
*/


static void EDMA_Configuration(void);

static void DMAMUX_Configuration(void);

static void PIT_Configuration(void);

static void DAC_Configuration(void);

static void Edma_Callback(edma_handle_t *DMA_Handle, void *userData, bool transferDone, uint32_t tcds);

static edma_handle_t DMA_Handle;             /* Edma DMA_Handler */

static edma_transfer_config_t transferConfig; /* Edma transfer config. */


status_t Audio_Init()
{
	/* Initialize DMAMUX. */
	DMAMUX_Configuration();

	/* Initialize EDMA. */
	EDMA_Configuration();

	/* Initialize the HW trigger source. */

	PIT_Configuration();
	PIT_StartTimer(AUDIO_PIT,AUDIO_PIT_CHNL);

	/* Initialize DAC. */
	DAC_Configuration();

	return kStatus_Success;

}

void Audio_ResetBuffers()
{
	for(int i=0; i<CIRC_BUFFER_LEN; i++)
	{
		memset(audioFrame[i].samples,0,AUDIO_BUFFER_SIZE);
		audioFrame[i].nSamples = AUDIO_BUFFER_SIZE;
		audioFrame[i].sampleRate = 44100;
	}
}

void Audio_Play()
{
	EDMA_StartTransfer(&DMA_Handle);

}

void Audio_Stop()
{
	EDMA_AbortTransfer(&DMA_Handle);
	//PIT_StopTimer(AUDIO_PIT,AUDIO_PIT_CHNL);
	Audio_ResetBuffers();
}

void Audio_Pause()
{
	EDMA_StopTransfer(&DMA_Handle);
	//PIT_StopTimer(AUDIO_PIT,AUDIO_PIT_CHNL);
}

void Audio_Resume()
{
	//PIT_StartTimer(AUDIO_PIT,AUDIO_PIT_CHNL);
	EDMA_StartTransfer(&DMA_Handle);
}
uint16_t * Audio_GetBackBuffer()
{
	return audioFrame[DMA_Handle.tail].samples;
}


uint32_t Audio_GetCurrentFrameNumber()
{
	NVIC_DisableIRQ(AUDIO_DMA_IRQ_ID);
	uint32_t n = audioFrame[DMA_Handle.header].frameNumber;
	NVIC_EnableIRQ(AUDIO_DMA_IRQ_ID);
	return n;
}

void Audio_FillBackBuffer(int16_t* samples, uint16_t nSamples, uint32_t sampleRate, uint32_t frameNumber)
{
	// Average stereo channels
	for(int i=0; i<nSamples; i++)
	{
		uint16_t L = ((uint16_t)(samples[2*i]+32768))>>4;
		uint16_t R = ((uint16_t)(samples[2*i+1]+32768))>>4;
		audioFrame[DMA_Handle.tail].samples[i] = L/2 + R/2;
		//audioFrame[DMA_Handle.tail].samples[i] = ((uint16_t)(samples[i]+32768))>>4;

	}

	audioFrame[DMA_Handle.tail].nSamples = nSamples/2;
	audioFrame[DMA_Handle.tail].sampleRate = sampleRate;
	audioFrame[DMA_Handle.tail].frameNumber = frameNumber;

	EDMA_PrepareTransfer(&transferConfig,
							 (void *)(audioFrame[DMA_Handle.tail].samples),
							 sizeof(uint16_t),
							 (void *)DAC_DATA_REG_ADDR,
							 sizeof(uint16_t),
							 sizeof(uint16_t),	// One sample per request
							 audioFrame[DMA_Handle.tail].nSamples * sizeof(uint16_t),// Transfer an entire frame
							 kEDMA_MemoryToPeripheral);

    EDMA_SubmitTransfer(&DMA_Handle, &transferConfig);
}

void Audio_SetSampleRate(uint32_t sr)
{
	PIT_SetTimerPeriod(AUDIO_PIT, AUDIO_PIT_CHNL, CLOCK_GetFreq(kCLOCK_BusClk)/sr+1);
}

bool Audio_BackBufferIsFree()
{
	NVIC_DisableIRQ(AUDIO_DMA_IRQ_ID);
	//bool b = DMA_Handle.tcdUsed < DMA_Handle.tcdSize;
	bool b = (DMA_Handle.tail+1)%DMA_Handle.tcdSize != DMA_Handle.header;
	NVIC_EnableIRQ(AUDIO_DMA_IRQ_ID);
	return b;
}

bool Audio_BackBufferIsEmpty()
{
	NVIC_DisableIRQ(AUDIO_DMA_IRQ_ID);
	bool b = (DMA_Handle.tcdUsed == 0);
	NVIC_EnableIRQ(AUDIO_DMA_IRQ_ID);
	return b;
}

static void Edma_Callback(edma_handle_t *handle, void *userData, bool transferDone, uint32_t tcds)
{
	if(transferDone==false)
	{

	}
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

    EDMA_Init(AUDIO_DMA_BASEADDR, &userConfig);

    EDMA_CreateHandle(&DMA_Handle, AUDIO_DMA_BASEADDR, AUDIO_DMA_CHANNEL);

    EDMA_SetCallback(&DMA_Handle, Edma_Callback, NULL);

    EDMA_InstallTCDMemory(&DMA_Handle, tcdMemoryPoolPtr, TCD_QUEUE_SIZE);

    /* Enable interrupt when transfer is done. */
    EDMA_EnableChannelInterrupts(AUDIO_DMA_BASEADDR, AUDIO_DMA_CHANNEL, kEDMA_MajorInterruptEnable);

}

static void DMAMUX_Configuration(void)
{
	 /* Configure DMAMUX */
	DMAMUX_Init(DMAMUX);

    DMAMUX_SetSource(DMAMUX, AUDIO_DMA_CHANNEL, AUDIO_DMA_ALWAYS_ENABLE_SOURCE);
    DMAMUX_EnablePeriodTrigger(DMAMUX, AUDIO_DMA_CHANNEL);

    DMAMUX_EnableChannel(DMAMUX, AUDIO_DMA_CHANNEL);
}


void PIT_Configuration()
{
	pit_config_t config;
	config.enableRunInDebug = false;
	PIT_Init(AUDIO_PIT, &config);
}

static void DAC_Configuration(void)
{
    dac_config_t dacConfigStruct;
    DAC_GetDefaultConfig(&dacConfigStruct);
    DAC_Init(AUDIO_DAC, &dacConfigStruct);
    DAC_Enable(AUDIO_DAC, true); /* Enable output. */

}


#endif
