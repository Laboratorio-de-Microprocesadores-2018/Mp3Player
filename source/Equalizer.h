/**
 * @file Equalizer.h
 * @brief
 *
 *
 */

#ifndef EQUALIZER_H_
#define EQUALIZER_H_


#include <stdbool.h>
#include <stdint.h>

#include "EqualizerDefs.h"


void Equalizer_Enable(bool b);

bool Equalizer_IsEnable();

void Equalizer_GetLevelLimits(int8_t *min, int8_t *max);

void Equalizer_SetLevel(uint8_t band, int8_t level);

int8_t Equalizer_GetLevel(uint8_t band);

void Equalizer_Init();

void Equalizer_Apply(int16_t * inputQ15,uint16_t size);



#endif

