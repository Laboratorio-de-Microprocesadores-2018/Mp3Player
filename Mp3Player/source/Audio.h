#ifndef AUDIO_H_
#define AUDIO_H_
#include "fsl_common.h"

#define CIRC_BUFFER_LEN 2
#define SAMPLES_PER_FRAME 1024 /////////// HAY QUE VER DONDE SE DEFINE O SI ES ASI


status_t Audio_Init();


void Audio_Play();

uint16_t * Audio_GetBackBuffer();

bool Audio_BackBufferIsFree();

void Audio_FillBackBuffer(int16_t* samples);

void Audio_SetSampleRate(uint16_t sr);

void Audio_Pause();

void Audio_Resume();



#endif /* AUDIO_H_ */
