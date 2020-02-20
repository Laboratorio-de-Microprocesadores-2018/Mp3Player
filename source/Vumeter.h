/**
 * @file Vumeter.h
 * @brief
 *
 *
 */

#ifndef VUMETER_H_
#define VUMETER_H_
#include "fsl_common.h"

#define NSAMPLES 1024
#define NBINS 8


void Vumeter_Init(void);
void Vumeter_Deinit(void);
void Vumeter_Generate(int16_t * s);
void Vumeter_Display(void);
void Vumeter_Clear(void);
bool Vumeter_BackBufferEmpty(void);
void Vumeter_RotateLeft();
void Vumeter_RotateRight();
#endif /* VUMETER_H_ */
