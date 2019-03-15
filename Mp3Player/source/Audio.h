#ifndef AUDIO_H_
#define AUDIO_H_
#include "fsl_common.h"


status_t Audio_Init();


void Audio_ResetBuffers();

void Audio_Play();

uint16_t * Audio_GetBackBuffer();

bool Audio_BackBufferIsFree();

bool Audio_BackBufferIsEmpty();

void Audio_FillBackBuffer(int16_t* samples, uint16_t nSamples, uint32_t sampleRate, uint32_t frameNumber);

void Audio_SetSampleRate(uint32_t sr);

void Audio_Pause();

void Audio_Stop();

void Audio_Resume();

uint32_t Audio_GetCurrentFrameNumber();


#endif /* AUDIO_H_ */
