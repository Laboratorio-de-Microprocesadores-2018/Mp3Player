/**
 * @file AudioDAC.c
 * @brief
 *
 *
 */

#include "Audio.h"

#if AUDIO_OUTPUT == DAC_OUTPUT

#include "fsl_dac.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"
#include "fsl_pit.h"
#include "fsl_debug_console.h"




#define AUDIO_DMA_CHANNEL 			0U
#define AUDIO_DMA_DAC_SOURCE 		45U
#define AUDIO_DMA_BASEADDR 			DMA0
#define DAC_DATA_REG_ADDR 			0x400cc000U //DAC_GetBufferAddress(AUDIO_DAC)
#define AUDIO_DMA_IRQ_ID 			DMA0_IRQn

#define AUDIO_PIT_CHNL  			kPIT_Chnl_0 // AUDIO_DMA_CHANNEL tiene que ser igual al numero de PIT!!


typedef struct{
	uint16_t samples[AUDIO_BUFFER_SIZE];
	uint16_t nSamples;
	uint32_t sampleRate;
	uint32_t frameNumber;
}AudioFrame_t;


static edma_handle_t DMA_Handle;
static AudioFrame_t audioQueue[CIRC_BUFFER_LEN];
volatile uint8_t queueDriver;
volatile uint8_t queueUser;

static uint8_t volumeLevel;

AT_NONCACHEABLE_SECTION_ALIGN(static edma_tcd_t tcdMemoryPoolPtr[(CIRC_BUFFER_LEN+1)], sizeof(edma_tcd_t));


static void Edma_Callback(edma_handle_t *DMA_Handle, void *userData, bool transferDone, uint32_t tcds);


status_t Audio_Init()
{
	/* Initialize DMAMUX. */
	DMAMUX_Init(DMAMUX);
	DMAMUX_SetSource(DMAMUX, AUDIO_DMA_CHANNEL, kDmaRequestMux0AlwaysOn58);
	DMAMUX_EnableChannel(DMAMUX, AUDIO_DMA_CHANNEL);
	DMAMUX_EnablePeriodTrigger(DMAMUX, AUDIO_DMA_CHANNEL);

	edma_channel_Preemption_config_t premptionConfig;
	premptionConfig.channelPriority=15;
	premptionConfig.enableChannelPreemption = false;
	premptionConfig.enablePreemptAbility = true;
	EDMA_SetChannelPreemptionConfig(DMA0,AUDIO_DMA_CHANNEL,&premptionConfig);


	/* Initialize EDMA. */
    EDMA_CreateHandle(&DMA_Handle, AUDIO_DMA_BASEADDR, AUDIO_DMA_CHANNEL);
    EDMA_InstallTCDMemory(&DMA_Handle, tcdMemoryPoolPtr, CIRC_BUFFER_LEN);
	EDMA_SetCallback(&DMA_Handle, Edma_Callback, NULL);
	//EDMA_EnableChannelInterrupts(DMA0,AUDIO_DMA_CHANNEL,kEDMA_ErrorInterruptEnable);//EDMA_EnableChannelInterrupts(AUDIO_DMA_BASEADDR, AUDIO_DMA_CHANNEL, kEDMA_MajorInterruptEnable);


	/* Initialize DAC. */
    dac_config_t dacConfigStruct;
    DAC_GetDefaultConfig(&dacConfigStruct);
    DAC_Init(DAC0, &dacConfigStruct);
    DAC_Enable(DAC0, true); /* Enable output. */

	/* Initialize the HW trigger source. */
	pit_config_t pitConfig;
	pitConfig.enableRunInDebug = false;
	PIT_Init(PIT, &pitConfig);
	PIT_StartTimer(PIT,AUDIO_PIT_CHNL);



	volumeLevel = Audio_GetMaxVolume()*3/4;

	return kStatus_Success;

}
void Audio_Deinit()
{
	DMAMUX_Deinit(DMAMUX);
	DAC_Deinit(DAC0);
	PIT_Deinit(PIT);
}

void Audio_Play()
{


}

void Audio_Stop()
{

	EDMA_AbortTransfer(&DMA_Handle);

    /* Clear all the internal information */
    for(int i=0; i<CIRC_BUFFER_LEN; i++)
    {
    	audioQueue[i].samples[0]=0;
    	audioQueue[i].nSamples = 0;
    }

    queueUser   = 0U;
    queueDriver = 0U;

	PIT_StopTimer(PIT,AUDIO_PIT_CHNL);
}

void Audio_Pause()
{
	EDMA_StopTransfer(&DMA_Handle);
}

void Audio_Resume()
{
	EDMA_StartTransfer(&DMA_Handle);
}

uint16_t * Audio_GetBackBuffer()
{
	return &audioQueue[queueUser].samples[0];
}


uint32_t Audio_GetCurrentFrameNumber()
{
	return audioQueue[queueDriver].frameNumber;

}

bool Audio_PushFrame(int16_t* samples, uint16_t nSamples, uint8_t nChans, uint32_t sampleRate, uint32_t frameNumber)
{
	if (audioQueue[queueUser].nSamples != 0)
	{
		return false;
	}

	// Fill buffer and apply volume
	if(nChans==2)
	{
		for(int i=0; i<(nSamples/2); i++)
		{
			uint16_t L = ((uint16_t)(samples[2*i]+32768))>>4;
			uint16_t R = ((uint16_t)(samples[2*i+1]+32768))>>4;

			uint8_t n = (19-volumeLevel)>>1;
			uint16_t temp = (L/2 + R/2)>>n;

			if(volumeLevel&1)
			{
				audioQueue[queueUser].samples[i] = (temp>>1) + (temp>>3) + (temp>>4) + (temp>>6);
			}
			else
			{
				audioQueue[queueUser].samples[i] = temp;
			}
		}
	}
	else if(nChans == 1)
	{
		for(int i=0; i<nSamples; i++)
		{
			audioQueue[queueUser].samples[i] = ((uint16_t)(samples[i]+32768))>>4;
		}
	}
	else
	{
		PRINTF("Audio_PushFrame() Invalid number of channels: %d\n",nChans);
	}

	audioQueue[queueUser].nSamples = nSamples/nChans;
	audioQueue[queueUser].sampleRate = sampleRate;
	audioQueue[queueUser].frameNumber = frameNumber;
	queueUser = (queueUser + 1U) % CIRC_BUFFER_LEN;

    edma_transfer_config_t config = {0};
	EDMA_PrepareTransfer(&config,
							 (void *)(audioQueue[DMA_Handle.tail].samples),
							 sizeof(uint16_t),
							 (void *)&DAC0->DAT[0],
							 sizeof(uint16_t),
							 sizeof(uint16_t),	// One sample per request
							 audioQueue[DMA_Handle.tail].nSamples * sizeof(uint16_t),// Transfer an entire frame
							 kEDMA_MemoryToPeripheral);

    if(EDMA_SubmitTransfer(&DMA_Handle, &config) != kStatus_Success)
    {
    	return false;
    }

    /* Start DMA transfer */
    EDMA_StartTransfer(&DMA_Handle);
    PIT_StartTimer(PIT,AUDIO_PIT_CHNL);

    return true;

}

void Audio_SetSampleRate(uint32_t sr)
{
	assert(sr!=0);

	PIT_SetTimerPeriod(PIT, AUDIO_PIT_CHNL, CLOCK_GetFreq(kCLOCK_BusClk)/sr+1);
}

bool Audio_QueueIsFree()
{
	return audioQueue[queueUser].nSamples == 0;
//	NVIC_DisableIRQ(AUDIO_DMA_IRQ_ID);
//	//bool b = DMA_Handle.tcdUsed < DMA_Handle.tcdSize;
//	bool b = (DMA_Handle.tail+1)%DMA_Handle.tcdSize != DMA_Handle.header;
//	NVIC_EnableIRQ(AUDIO_DMA_IRQ_ID);
//	return b;

	//return EDMA_GetUnusedTCDNumber(&DMA_Handle) != 0;


}

bool Audio_QueueIsEmpty()
{
	return audioQueue[queueDriver].nSamples == 0;
//	NVIC_DisableIRQ(AUDIO_DMA_IRQ_ID);
//	bool b = (DMA_Handle.tcdUsed == 0);
//	NVIC_EnableIRQ(AUDIO_DMA_IRQ_ID);
//	return b;

	//return EDMA_GetUnusedTCDNumber(&DMA_Handle) == 0;
}

static void Edma_Callback(edma_handle_t *handle, void *userData, bool transferDone, uint32_t tcds)
{
	/* Mark a frame as completed. */
	audioQueue[queueDriver].samples[0] = 0;
	audioQueue[queueDriver].nSamples = 0;

	queueDriver = (queueDriver + 1U) % CIRC_BUFFER_LEN;

//	if (callback != NULL)
//	{
//		(callback)(privHandle->base, saiHandle, kStatus_SAI_TxIdle, saiHandle->userData);
//	}

	/* If all data finished, just stop the transfer */
	if (audioQueue[queueDriver].nSamples == 0)
	{
		//PRINTF("Queue has emptied!\n");
		PIT_StopTimer(PIT,AUDIO_PIT_CHNL);
		EDMA_AbortTransfer(&DMA_Handle);
	}
	else
	{
		//Audio_SetSampleRate(audioQueue[queueDriver].sampleRate);
	}

}


void Audio_SetVolume(uint8_t level)
{
	assert(level>0 && level<20);

	volumeLevel = level;

}
uint8_t Audio_GetVolume(void)
{
	return volumeLevel;
}

uint8_t Audio_GetMaxVolume(void)
{
	return 19;
}


/*

#include <stdio.h>
#include <stdint.h>

int main()
{
    int16_t sample = 0x7FFF;
    int8_t N = 20;
    PRINTF("levels=[");
	for(uint8_t volume=0; volume<N; volume++)
    {
        uint8_t n = volume>>1;
        int16_t temp;
        // For odd volume level divide by 2^n*10^(3/20)
        // This is aproximatelly 0.101101 in binary
        if(volume&1)
        {
            temp = sample >> n;
            temp = (temp>>1) + (temp>>3) + (temp>>4) + (temp>>6);
        }
        // For even volume level divide by 2^n
        else
        {
            temp = sample>>n;
        }
        if(volume==N-1)
            PRINTF("%d]; ",temp);
        else
            PRINTF("%d, ",temp);


    }
    return 0;
}
*/

void applyVolume(int16_t * samples, uint16_t nSamples, const uint8_t volume)
{
	// Invert scale: volume=0 minimum volume, volume=19 maximum volume
	uint8_t n = (19-volume)>>1;

	// For odd volume level divide by 2^n*10^(3/20)
	// This is approximately 0.101101 in binary
	if(volume&1)
	{
		while(nSamples--)
		{
			int16_t temp = (*samples++) >> n;
			(*samples++) = (temp>>1) + (temp>>3) + (temp>>4) + (temp>>6);
		}
	}
	// For even volume level divide by 2^n
	else
	{
		while(nSamples--)
		{
			*samples = (*samples++)>>n;
		}
	}
}


#endif
