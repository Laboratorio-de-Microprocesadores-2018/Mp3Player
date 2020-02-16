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

#define SAI_XFER_QUEUE_SIZE CIRC_BUFFER_LEN

//#include "fsl_sai_edma.h"
#include "fsl_dmamux.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_edma.h"
#include "fsl_sai.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "LM49450.h"


#define AUDIO_SAI 			I2S0
#define AUDIO_DMA 			DMA0
#define AUDIO_DMA_CHANNEL	0
#define AUDIO_DMA_IRQ_ID 	DMA0_IRQn

typedef struct{
	uint16_t samples[AUDIO_BUFFER_SIZE];
	uint16_t nSamples;
	uint32_t sampleRate;
	uint32_t frameNumber;
}audioQueue_t;

static edma_handle_t DMA_Handle;
static audioQueue_t audioQueue[CIRC_BUFFER_LEN];
volatile uint8_t queueDriver;
volatile uint8_t queueUser;

//static sai_handle_t SAI_Handle;
//static sai_handle_t SAI_Handle;
static sai_transfer_format_t SAI_TransferFormat;

/**
 *
 */
//static void SAI_Callback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData);
//static void Edma_Callback(edma_handle_t *handle, void *userData, bool transferDone, uint32_t tcds)

AT_NONCACHEABLE_SECTION_ALIGN(static edma_tcd_t tcdMemoryPoolPtr[(CIRC_BUFFER_LEN+1)], sizeof(edma_tcd_t));
static void Edma_Callback(edma_handle_t *DMA_Handle, void *userData, bool transferDone, uint32_t tcds);



status_t Audio_Init()
{
	//////////////////////////////////////////////////////////
	// Sets up the dma channel to transfer
	DMAMUX_Init(DMAMUX0);
	DMAMUX_SetSource(DMAMUX0, AUDIO_DMA_CHANNEL, kDmaRequestMux0I2S0Tx);
	DMAMUX_EnableChannel(DMAMUX0, AUDIO_DMA_CHANNEL);

//	edma_channel_Preemption_config_t premptionConfig;
//	premptionConfig.channelPriority=15;
//	premptionConfig.enableChannelPreemption = false;
//	premptionConfig.enablePreemptAbility = true;
//	EDMA_SetChannelPreemptionConfig(DMA0,AUDIO_DMA_CHANNEL,&premptionConfig);

	EDMA_CreateHandle(&DMA_Handle, AUDIO_DMA, AUDIO_DMA_CHANNEL);
	EDMA_InstallTCDMemory(&DMA_Handle, tcdMemoryPoolPtr, CIRC_BUFFER_LEN);
	EDMA_SetCallback(&DMA_Handle, Edma_Callback, NULL);

	//////////////////////////////////////////////////////////
	// Initialize SAI.
	sai_config_t config;
	SAI_TxGetDefaultConfig(&config);
	config.mclkSource = kSAI_MclkSourceSelect1;
	config.bclkSource  = kSAI_BclkSourceMclkDiv;
	config.masterSlave = kSAI_Master;
	config.mclkOutputEnable = true;
	config.protocol = kSAI_BusLeftJustified;
	config.syncMode = kSAI_ModeAsync;
	SAI_TxInit(AUDIO_SAI, &config);
	AUDIO_SAI->TCSR |= I2S_TCSR_DBGE_MASK;

	// Esta instala los TCDS, hecho arriba SAI_TransferTxCreateHandleEDMA(AUDIO_SAI, &SAI_Handle, SAI_Callback, NULL);


	sai_transceiver_t transceiverConfig;
	SAI_GetLeftJustifiedConfig(&transceiverConfig, kSAI_WordWidth16bits, kSAI_MonoLeft, kSAI_Channel0Mask);// | kSAI_Channel1Mask);
	transceiverConfig.bitClock.bclkSource = kSAI_BclkSourceMclkOption1;

	SAI_TxSetConfig(AUDIO_SAI, &transceiverConfig);

	sai_master_clock_t mclkConfig;
	mclkConfig.mclkHz = 12000000;
	mclkConfig.mclkOutputEnable = true;
	mclkConfig.mclkSource = kSAI_MclkSourceSelect1;
	mclkConfig.mclkSourceClkHz = 12000000;
	SAI_SetMasterClockConfig(AUDIO_SAI, &mclkConfig);

	/* Set sample rate*/
//	SAI_TxSetBitClockRate(AUDIO_SAI, 12000000,kSAI_SampleRate44100Hz, kSAI_WordWidth16bits, transceiverConfig.channelNums);


	LM49450_SlaveConfig LM49450config;
	LM49450_GetDefaultSlaveConfig(&LM49450config);
	LM49450config.oversampleRate = LM49450_DAC_OSR_125;
	LM49450config.I2sMode = LM49450_I2s_LeftJustified;
	LM49450config.defaultDacFilter = false;
	LM49450config.chargePumpDiv = 73;


	//LM49450config.reference = LM49450_InternalRef;

	//LM49450config.defaultDacFilter = true;
	//LM49450config.oscillatorMode = LM49450_FixedFrequency;
	//LM49450config.mute = false;
	//LM49450config.lineInEnable = false;
	//LM49450config.enable = true;
	//LM49450config.dither = LM49450_DitherDefault;
	//LM49450config.MclkDiv; /////////////////////////////////////////////////

	//LM49450config.wordSize = LM49450_I2sWordSize_16; // Solo se usa en right justified mode
	//LM49450config.stereoMode = LM49450_StereoNormal;
	//LM49450config.wordOrder = LM49450_WordOrderNormal;

// Solo en master mode!!
	//LM49450config.I2sClkDiv;/////////////////////////////////////////////////////
	//LM49450config.bitsPerWord = LM49450_I2sBitsPerWord_16; // Solo se usa en master mode

//	LM49450config.wordSelectLineMaster = false;
//	LM49450config.clockLineMaster = false;

	//LM49450config.headphone3D.enable = false;
	//LM49450config.speaker3D.enable = false;
	//config.lineInEnable = true;

	LM49450_SlaveInit(&LM49450config);

	LM49450_Enable(true);


	return kStatus_Success;

}

void Audio_Deinit(void)
{
	return;
}

void Audio_Play(/*uint32_t sampleRate,*/)
{
//	SAI_TxEnable(AUDIO_SAI, true);
// Esto lo hace el sdk!
//	/* Enable Tx */
//	SAI_TxEnable(AUDIO_SAI, true);
//
//	/* Enable DMA */
//	SAI_TxEnableDMA(AUDIO_SAI, kSAI_FIFORequestDMAEnable, true);

//	SAI_TransferFormat.sampleRate_Hz = 44100;
//	SAI_TransferFormat.bitWidth = 16;
//	SAI_TransferFormat.stereo = kSAI_MonoRight;
//	SAI_TransferFormat.masterClockHz = 60000;
//	SAI_TransferFormat.watermark = 4;
//
//	SAI_TransferTxSetFormatEDMA(AUDIO_SAI,&SAI_Handle,&SAI_TransferFormat,60000,2*16*44100/1000);
//	SAI_TxClearStatusFlags(AUDIO_SAI, kSAI_FIFOErrorFlag);
}


void Audio_Pause()
{
	/* Disable dma */
	EDMA_StopTransfer(&DMA_Handle);

	/* Disable DMA enable bit */
	SAI_TxEnableDMA(AUDIO_SAI, kSAI_FIFORequestDMAEnable, false);

	/* Disable Tx */
	SAI_TxEnable(AUDIO_SAI, false);
}

void Audio_Resume()
{
	/* Enable Tx */
	SAI_TxEnable(AUDIO_SAI, true);

	/* Enable DMA */
	SAI_TxEnableDMA(AUDIO_SAI, kSAI_FIFORequestDMAEnable, true);

	EDMA_StartTransfer(&DMA_Handle);

}

void Audio_Stop()
{
	 /* Disable dma */
	EDMA_AbortTransfer(&DMA_Handle);

	/* Disable the channel FIFO */
	SAI_TxSetChannelFIFOMask(AUDIO_SAI,0);

	/* Disable DMA enable bit */
	SAI_TxEnableDMA(AUDIO_SAI, kSAI_FIFORequestDMAEnable, false);

	/* Disable Tx */
	SAI_TxEnable(AUDIO_SAI, false);

	/* If Tx is disabled, reset the FIFO pointer and clear error flags */
	if ((AUDIO_SAI->TCSR & I2S_TCSR_TE_MASK) == 0UL)
	{
		AUDIO_SAI->TCSR |= (I2S_TCSR_FR_MASK | I2S_TCSR_SR_MASK);
		AUDIO_SAI->TCSR &= ~I2S_TCSR_SR_MASK;
	}

	queueDriver = 0;
	queueUser = 0;

	Audio_ResetQueue();
}


uint16_t * Audio_GetBackBuffer()
{
	return audioQueue[queueUser].samples;
}

void Audio_ResetQueue()
{
	for(int i=0; i<SAI_XFER_QUEUE_SIZE; i++)
	{
		audioQueue[i].samples[0] = 0;
		audioQueue[i].nSamples = 0;
	}
}


uint32_t Audio_GetCurrentFrameNumber()
{
	//NVIC_DisableIRQ(AUDIO_DMA_IRQ_ID);
	uint32_t n = audioQueue[queueDriver].frameNumber;
	//NVIC_EnableIRQ(AUDIO_DMA_IRQ_ID);
	return n;
}

bool Audio_PushFrame(int16_t* samples, uint16_t nSamples,uint8_t nChans, uint32_t sampleRate, uint32_t frameNumber)
{
	// Average stereo channels
	if(nChans==2)
	{
		for(int i=0; i<nSamples/2; i++)
		{
			//audioQueue[queueUser].samples[i] = samples[i];
			audioQueue[queueUser].samples[i] = samples[2*i];
		}
	}
	else if(nChans == 1)
	{
		for(int i=0; i<nSamples; i++)
		{
			audioQueue[queueUser].samples[i] = samples[i];
			//audioQueue[DMA_Handle.tail].samples[2*i] = samples[i];
		}
	}
	else
	{
		PRINTF("Audio_PushFrame() Invalid number of channels: %d\n",nChans);
	}

	audioQueue[queueUser].nSamples = nSamples/nChans; // STEREO
	audioQueue[queueUser].sampleRate = sampleRate;
	audioQueue[queueUser].frameNumber = frameNumber;

	edma_transfer_config_t config = {0};
	EDMA_PrepareTransfer(&config,
							 (void *)(audioQueue[queueUser].samples),
							 sizeof(uint16_t),
							 (void *)SAI_TxGetDataRegisterAddress(I2S0,0),
							 sizeof(uint16_t), /*  sizeof(uint32_t) STEREO */
							 sizeof(uint16_t),
							 audioQueue[queueUser].nSamples * sizeof(uint16_t),// Transfer an entire frame
							 kEDMA_MemoryToPeripheral); // kEDMA_MemoryToPeripheral STEREO

// ESTO ES PARA STEREO
	//EDMA_SetMinorOffsetConfig(AUDIO_DMA,AUDIO_DMA_CHANNEL, minorOffsetConfig);
//	config.minorLoopBytes |= DMA_NBYTES_MLOFFYES_DMLOE_MASK; // Enable destination minor loop offset
//	config.minorLoopBytes &= ~DMA_NBYTES_MLOFFYES_SMLOE_MASK;// Disable source minor loop offset
//	config.minorLoopBytes &= ~DMA_NBYTES_MLOFFYES_MLOFF_MASK;
//	config.minorLoopBytes |= DMA_NBYTES_MLOFFYES_MLOFF(-4);

	status_t s = EDMA_SubmitTransfer(&DMA_Handle, &config);

//	edma_minor_offset_config_t minorOffsetConfig;
//	minorOffsetConfig.enableDestMinorOffset = true;
//	minorOffsetConfig.enableSrcMinorOffset = false;
//	minorOffsetConfig.minorOffset = -4;
//	EDMA_TcdSetMinorOffsetConfig(&DMA_Handle.tcdPool[DMA_Handle.tail], minorOffsetConfig);


	queueUser = (queueUser + 1U) % CIRC_BUFFER_LEN;

	if(s == kStatus_Success)
	{
		/* Start DMA transfer */
		EDMA_StartTransfer(&DMA_Handle);

		 /* Enable DMA enable bit */
		SAI_TxEnableDMA(AUDIO_SAI, kSAI_FIFORequestDMAEnable, true);

		/* Enable SAI Tx clock */
		SAI_TxEnable(AUDIO_SAI, true);

		/* Enable the channel FIFO */
		SAI_TxSetChannelFIFOMask(AUDIO_SAI, kSAI_Channel0Mask); //   STEREO

		return kStatus_Success;
	}

	return s;


//
//	sai_transfer_t transfer;
//	transfer.data = (uint8_t*)audioQueue[SAI_Handle.queueUser].samples;
//	transfer.dataSize = nSamples*sizeof(samples[0]);

//	status_t s = SAI_TransferSendNonBlocking(AUDIO_SAI, &SAI_Handle, &transfer);
//	size_t count;
//	while(SAI_TransferGetSendCount(AUDIO_SAI,&SAI_Handle,&count) != kStatus_NoTransferInProgress);

//
//	status_t s = SAI_TransferSendEDMA(AUDIO_SAI,&SAI_Handle,&transfer);
//
//	if(s==kStatus_Success)
//		return true;
//	else
//		return false;


}

void Audio_SetSampleRate(uint32_t sr)
{
	LM49450_SetSampleRate(12000000,sr);

//	sai_transfer_format_t format;
//	format.bitWidth = kSAI_WordWidth16bits;
//	format.channelMask = kSAI_Channel0Mask;
//	format.isFrameSyncCompact = true;
//	format.masterClockHz = 12000000;
//	format.protocol = kSAI_BusLeftJustified;
//	format.sampleRate_Hz = sr;
//	format.stereo = kSAI_MonoLeft;
//	format.watermark = 4;
	//SAI_TxSetFormat(AUDIO_SAI, &format, 12000000, 12000000);


	SAI_TxSetBitClockRate(AUDIO_SAI, 12000000,sr, kSAI_WordWidth16bits, 2);

}

bool Audio_QueueIsFree()
{
	return audioQueue[queueUser].nSamples == 0;
//	NVIC_DisableIRQ(AUDIO_DMA_IRQ_ID);
//	bool b = SAI_Handle.saiQueue[SAI_Handle.queueUser].data == NULL;
//	NVIC_EnableIRQ(AUDIO_DMA_IRQ_ID);
//	return b;
}

bool Audio_QueueIsEmpty()
{
	return audioQueue[queueDriver].nSamples == 0;
//	NVIC_DisableIRQ(AUDIO_DMA_IRQ_ID);
//	bool b = SAI_Handle.saiQueue[SAI_Handle.queueDriver].data == NULL;
//	NVIC_EnableIRQ(AUDIO_DMA_IRQ_ID);
//	return b;
}



bool Audio_AreHeadphonesPlugged()
{
	return GPIO_PinRead(HPL_SENSE_GPIO,HPR_SENSE_PIN) || GPIO_PinRead(HPL_SENSE_GPIO,HPL_SENSE_PIN);
}

void Audio_SetVolume(uint8_t level)
{

	if(Audio_AreHeadphonesPlugged())
	{
		LM49450_SetHpVolume(level);
	}
	else
	{
		LM49450_SetSpkVolume(level);
	}
}
uint8_t Audio_GetVolume(void)
{
	if(Audio_AreHeadphonesPlugged())
	{
		return LM49450_GetHpVolume();
	}
	else
	{
		return LM49450_GetSpkVolume();
	}
}
uint8_t Audio_GetMaxVolume(void)
{
	return 31;
}


//static void SAI_Callback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
//{
//	int a = 8;
//}


static void Edma_Callback(edma_handle_t *handle, void *userData, bool transferDone, uint32_t tcds)
{
	/* Mark a frame as completed. */
	audioQueue[queueDriver].samples[0] = 0;
	audioQueue[queueDriver].nSamples = 0;

	queueDriver = (queueDriver + 1U) % CIRC_BUFFER_LEN;

	/* If all data finished, just stop the transfer */
	if (audioQueue[queueDriver].nSamples == 0)
	{
		SAI_TxEnableDMA(AUDIO_SAI, kSAI_FIFORequestDMAEnable, false);
		EDMA_AbortTransfer(&DMA_Handle);
	}
	else
	{
		//Audio_SetSampleRate(audioQueue[queueDriver].sampleRate);
	}

}



#endif
