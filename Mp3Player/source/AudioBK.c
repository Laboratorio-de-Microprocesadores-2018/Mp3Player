///*#include "Audio.h"
//#include "fsl_dac.h"
//#include "fsl_edma.h"
//#include "fsl_pdb.h"
//#include "fsl_dmamux.h"
//#include "fsl_pit.h"
//#include "assert.h"
//
//#define  AUDIO_BUFFER_SIZE 2304
//#define  CIRC_BUFFER_LEN 10
//
//#define USE_PIT 1
//
//#define AUDIO_PDB_BASEADDR PDB0
//#define AUDIO_PDB_MODULUS_VALUE 0x1FFU
//#define AUDIO_PDB_DELAY_VALUE 0U
//#define AUDIO_PDB_DAC_CHANNEL kPDB_DACTriggerChannel0
//#define AUDIO_PDB_DAC_INTERVAL_VALUE 0x1FFU
//
//#define AUDIO_DAC DAC0
//#define AUDIO_DMA_CHANNEL 0U
//#define AUDIO_DMA_DAC_SOURCE 45U
//#define AUDIO_DMA_BASEADDR DMA0
//#define DAC_DATA_REG_ADDR 0x400cc000U //DAC_GetBufferAddress(AUDIO_DAC)
//#define AUDIO_DMA_IRQ_ID DMA0_IRQn
//
//
//// PIT VERSION   AUDIO_DMA_CHANNEL tiene que ser igual al numero de PIT!!
//#define AUDIO_DMA_ALWAYS_ENABLE_SOURCE 58U
//#define AUDIO_PIT PIT
//#define AUDIO_PIT_CHNL  kPIT_Chnl_0
//
//typedef struct{
//	uint16_t samples[AUDIO_BUFFER_SIZE];
//	uint16_t nSamples;
//	uint32_t sampleRate;
//	uint32_t frameNumber;
//}PCM_AudioFrame;
//
//static PCM_AudioFrame audioFrame[CIRC_BUFFER_LEN+1];
//static volatile uint8_t circBufferHead;
//static volatile uint8_t circBufferTail;
//
//// Circular buffer
//#define BUFFER_POP  circBufferTail = (circBufferTail+1)%CIRC_BUFFER_LEN
//#define BUFFER_PUSH circBufferHead = (circBufferHead+1)%CIRC_BUFFER_LEN
//#define BUFFER_IS_EMPTY (circBufferHead == circBufferTail)
//#define BUFFER_IS_FULL (((circBufferHead+1)%CIRC_BUFFER_LEN)==circBufferTail)
//
//static void EDMA_Configuration(void);
//
//static void DMAMUX_Configuration(void);
//
//static void PDB_Configuration(void);
//
//static void PIT_Configuration(void);
//
//static void DAC_Configuration(void);
//
//static void Edma_Callback(edma_handle_t *handle, void *userData, bool transferDone, uint32_t tcds);
//
//static edma_handle_t DMA_Handle;             /* Edma handler */
//
//static edma_transfer_config_t transferConfig; /* Edma transfer config. */
//
//static bool pausePending;
//
//status_t Audio_Init()
//{
//	/* Initialize DMAMUX. */
//	DMAMUX_Configuration();
//
//	/* Initialize EDMA. */
//	EDMA_Configuration();
//
//	/* Initialize the HW trigger source. */
//#if USE_PIT == 0
//	PDB_Configuration();
//#elif USE_PIT==1
//	PIT_Configuration();
//	PIT_StartTimer(AUDIO_PIT,AUDIO_PIT_CHNL);
//#endif
//	/* Initialize DAC. */
//	DAC_Configuration();
//
//#if USE_PIT == 0
//	/* Generate continuous trigger signal to DAC. */
//	PDB_DoSoftwareTrigger(AUDIO_PDB_BASEADDR);
//#endif
//
//	return kStatus_Success;
//
//}
//
//
//void Audio_ResetBuffers()
//{
//	for(int i=0; i<CIRC_BUFFER_LEN; i++)
//	{
//		memset(audioFrame[i].samples,0,AUDIO_BUFFER_SIZE);
//		audioFrame[i].nSamples = AUDIO_BUFFER_SIZE;
//		audioFrame[i].sampleRate = 44100;
//	}
//	circBufferHead=0;
//	circBufferTail=0;
//}
//
//void Audio_Play()
//{
//	EDMA_PrepareTransfer(&transferConfig,
//						 (void *)(audioFrame[circBufferTail].samples),
//						 sizeof(uint16_t),
//						 (void *)DAC_DATA_REG_ADDR,
//						 sizeof(uint16_t),
//						 sizeof(uint16_t),						// One sample per request
//						 AUDIO_BUFFER_SIZE * sizeof(uint16_t),	// Transfer an entire frame
//						 kEDMA_MemoryToPeripheral);
//
//	EDMA_SetTransferConfig(AUDIO_DMA_BASEADDR, AUDIO_DMA_CHANNEL, &transferConfig, NULL);
//
//	/* Enable transfer. */
//	EDMA_SubmitTransfer(&DMA_Handle, &transferConfig);
//	EDMA_StartTransfer(&DMA_Handle);
//
//	// Free back buffer
//	BUFFER_POP;
//
//	//PIT_StartTimer(AUDIO_PIT,AUDIO_PIT_CHNL);
//}
//
//void Audio_Stop()
//{
//	EDMA_AbortTransfer(&DMA_Handle);
//	//PIT_StopTimer(AUDIO_PIT,AUDIO_PIT_CHNL);
//	Audio_ResetBuffers();
//}
//
//void Audio_Pause()
//{
//	EDMA_StopTransfer(&DMA_Handle);
//	//PIT_StopTimer(AUDIO_PIT,AUDIO_PIT_CHNL);
//}
//
//void Audio_Resume()
//{
//	//PIT_StartTimer(AUDIO_PIT,AUDIO_PIT_CHNL);
//	EDMA_StartTransfer(&DMA_Handle);
//}
//uint16_t * Audio_GetBackBuffer()
//{
//	return audioFrame[circBufferTail].samples;
//}
//
//
//uint32_t Audio_GetCurrentFrameNumber()
//{
//	NVIC_DisableIRQ(AUDIO_DMA_IRQ_ID);
//	uint32_t n = audioFrame[circBufferTail].frameNumber;
//	NVIC_EnableIRQ(AUDIO_DMA_IRQ_ID);
//	return n;
//}
//
//void Audio_FillBackBuffer(int16_t* samples, uint16_t nSamples, uint32_t sampleRate, uint32_t frameNumber)
//{
//	for(int i=0; i<nSamples; i++)
//	{
//		audioFrame[circBufferHead].samples[i] = ((uint16_t)(samples[i]+32768))>>4;
//	}
//
//	audioFrame[circBufferHead].nSamples = nSamples;
//	audioFrame[circBufferHead].sampleRate = sampleRate;
//	audioFrame[circBufferHead].frameNumber = frameNumber;
//
//	// Make this push atomic operation
//	NVIC_DisableIRQ(AUDIO_DMA_IRQ_ID);
//	BUFFER_PUSH;
//	NVIC_EnableIRQ(AUDIO_DMA_IRQ_ID);
//}
//
//void Audio_SetSampleRate(uint32_t sr)
//{
//#if USE_PIT == 1
//	PIT_SetTimerPeriod(AUDIO_PIT, AUDIO_PIT_CHNL, CLOCK_GetFreq(kCLOCK_BusClk)/sr+1);
//#endif
//}
//
//
//
//bool Audio_BackBufferIsFree()
//{
//	NVIC_DisableIRQ(AUDIO_DMA_IRQ_ID);
//	bool b = !BUFFER_IS_FULL;
//	NVIC_EnableIRQ(AUDIO_DMA_IRQ_ID);
//	return b;
//}
//
//bool Audio_BackBufferIsEmpty()
//{
//	NVIC_DisableIRQ(AUDIO_DMA_IRQ_ID);
//	bool b = !BUFFER_IS_EMPTY;
//	NVIC_EnableIRQ(AUDIO_DMA_IRQ_ID);
//	return b;
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//static void EDMA_Configuration(void)
//{
//    edma_config_t userConfig;
//    EDMA_GetDefaultConfig(&userConfig);
//    userConfig.enableRoundRobinArbitration = false;
//    userConfig.enableHaltOnError = true;
//    userConfig.enableContinuousLinkMode = false;
//    userConfig.enableDebugMode = true;
//
//    EDMA_Init(AUDIO_DMA_BASEADDR, &userConfig);
//
//    EDMA_CreateHandle(&DMA_Handle, AUDIO_DMA_BASEADDR, AUDIO_DMA_CHANNEL);
//    EDMA_SetCallback(&DMA_Handle, Edma_Callback, NULL);
//
//   /* EDMA_PrepareTransfer(&transferConfig, (void *)(samples + index), sizeof(uint16_t),
//                         (void *)DAC_DATA_REG_ADDR, sizeof(uint16_t), DAC_DATL_COUNT * sizeof(uint16_t),
//                         DAC_DATL_COUNT * sizeof(uint16_t), kEDMA_MemoryToMemory);
//    EDMA_SubmitTransfer(&DMA_Handle, &transferConfig);
//*/
//
//    /* Enable interrupt when transfer is done. */
//    EDMA_EnableChannelInterrupts(AUDIO_DMA_BASEADDR, AUDIO_DMA_CHANNEL, kEDMA_MajorInterruptEnable);
//
//}
//
//static void DMAMUX_Configuration(void)
//{
//	 /* Configure DMAMUX */
//	DMAMUX_Init(DMAMUX);
//#if USE_PIT == 0
//    DMAMUX_SetSource(DMAMUX, AUDIO_DMA_CHANNEL, AUDIO_DMA_DAC_SOURCE);
//#elif USE_PIT == 1
//    DMAMUX_SetSource(DMAMUX, AUDIO_DMA_CHANNEL, AUDIO_DMA_ALWAYS_ENABLE_SOURCE);
//    DMAMUX_EnablePeriodTrigger(DMAMUX, AUDIO_DMA_CHANNEL);
//#endif
//    DMAMUX_EnableChannel(DMAMUX, AUDIO_DMA_CHANNEL);
//}
//
///* Enable the trigger source of PDB. */
//static void PDB_Configuration(void)
//{
//    pdb_config_t pdbConfigStruct;
//
//    /*
//     * pdbConfigStruct.loadValueMode = kPDB_LoadValueImmediately;
//     * pdbConfigStruct.prescalerDivider = kPDB_PrescalerDivider1;
//     * pdbConfigStruct.dividerMultiplicationFactor = kPDB_DividerMultiplicationFactor40;
//     * pdbConfigStruct.triggerInputSource = kPDB_TriggerSoftware;
//     * pdbConfigStruct.enableContinuousMode = true;
//     */
//
//    PDB_GetDefaultConfig(&pdbConfigStruct);
//    pdbConfigStruct.dividerMultiplicationFactor = kPDB_DividerMultiplicationFactor1;
//    pdbConfigStruct.enableContinuousMode = true;
//    PDB_Init(AUDIO_PDB_BASEADDR, &pdbConfigStruct);
//    PDB_EnableInterrupts(AUDIO_PDB_BASEADDR, kPDB_DelayInterruptEnable);
//    PDB_SetModulusValue(AUDIO_PDB_BASEADDR, AUDIO_PDB_MODULUS_VALUE);
//    PDB_SetCounterDelayValue(AUDIO_PDB_BASEADDR, AUDIO_PDB_DELAY_VALUE);
//
//    /* Set DAC trigger. */
//    pdb_dac_trigger_config_t pdbDacTriggerConfigStruct;
//    pdbDacTriggerConfigStruct.enableExternalTriggerInput = false;
//    pdbDacTriggerConfigStruct.enableIntervalTrigger = true;
//    PDB_SetDACTriggerConfig(AUDIO_PDB_BASEADDR, AUDIO_PDB_DAC_CHANNEL, &pdbDacTriggerConfigStruct);
//    PDB_SetDACTriggerIntervalValue(AUDIO_PDB_BASEADDR, AUDIO_PDB_DAC_CHANNEL, AUDIO_PDB_DAC_INTERVAL_VALUE);
//
//    /* Load PDB values. */
//    PDB_DoLoadValues(AUDIO_PDB_BASEADDR);
//}
//
//void PIT_Configuration()
//{
//	pit_config_t config;
//	config.enableRunInDebug = false;
//	PIT_Init(AUDIO_PIT, &config);
//}
//
//static void DAC_Configuration(void)
//{
//    dac_config_t dacConfigStruct;
//    DAC_GetDefaultConfig(&dacConfigStruct);
//    DAC_Init(AUDIO_DAC, &dacConfigStruct);
//    DAC_Enable(AUDIO_DAC, true); /* Enable output. */
//
//    /* Configure the DAC buffer. */
//#if USE_PIT==0
//    DAC_EnableBuffer(AUDIO_DAC, true);
//    dac_buffer_config_t dacBufferConfigStruct;
//    DAC_GetDefaultBufferConfig(&dacBufferConfigStruct);
//    dacBufferConfigStruct.triggerMode = kDAC_BufferTriggerByHardwareMode;
//    DAC_SetBufferConfig(AUDIO_DAC, &dacBufferConfigStruct);
//    DAC_SetBufferReadPointer(AUDIO_DAC, 0U); /* Make sure the read pointer to the start. */
//
//    /* Enable DMA. */
//    DAC_EnableBufferInterrupts(AUDIO_DAC, kDAC_BufferReadPointerTopInterruptEnable);
//    DAC_EnableBufferDMA(AUDIO_DAC, true);
//#endif
//}
//
//
//static void Edma_Callback(edma_handle_t *handle, void *userData, bool transferDone, uint32_t tcds)
//{
//#if USE_PIT == 0
//	static uint16_t index;
//	static uint16_t * activeBuffer;
//
//    /* Clear Edma interrupt flag. */
//    EDMA_ClearChannelStatusFlags(AUDIO_DMA_BASEADDR, AUDIO_DMA_CHANNEL, kEDMA_InterruptFlag);
//
//    if(pausePending == true)
//    {
//    	pausePending = false;
//    	return;
//    }
//
//    /* Setup transfer */
//    // End of current buffer
//    if (index > SAMPLES_PER_FRAME)
//    {
//        BUFFER_POP;
//        activeBuffer = audioFrame[circBufferTail];
//        index = 0U;
//    }
//
//    EDMA_PrepareTransfer(&transferConfig,
//    					 (void *)(activeBuffer + index),
//						 sizeof(uint16_t),
//                         (void *)DAC_DATA_REG_ADDR,
//						 sizeof(uint16_t),
//						 DAC_DATL_COUNT * sizeof(uint16_t),
//                         DAC_DATL_COUNT * sizeof(uint16_t),
//						 kEDMA_MemoryToMemory);
//
//    EDMA_SetTransferConfig(AUDIO_DMA_BASEADDR, AUDIO_DMA_CHANNEL, &transferConfig, NULL);
//
//    /* Enable transfer. */
//    EDMA_StartTransfer(&DMA_Handle);
//
//    index += DAC_DATL_COUNT;
//#elif USE_PIT == 1
//
//    assert(BUFFER_IS_EMPTY==false);
//
//
//    Audio_SetSampleRate(audioFrame[circBufferTail].sampleRate);
//
//    EDMA_PrepareTransfer(&transferConfig,
//						 (void *)(audioFrame[circBufferTail].samples),
//						 sizeof(uint16_t),
//						 (void *)DAC_DATA_REG_ADDR,
//						 sizeof(uint16_t),
//						 sizeof(uint16_t),						// One sample per request
//						 audioFrame[circBufferTail].nSamples * sizeof(uint16_t),	// Transfer an entire frame
//						 kEDMA_MemoryToPeripheral);
//    BUFFER_POP;
//
//	EDMA_SetTransferConfig(AUDIO_DMA_BASEADDR, AUDIO_DMA_CHANNEL, &transferConfig, NULL);
//	//EDMA_SubmitTransfer(&DMA_Handle, &transferConfig);
//	/* Enable transfer. */
//	EDMA_StartTransfer(&DMA_Handle);
//#endif
//}
