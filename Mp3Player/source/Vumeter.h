#ifndef VUMETER_H_
#define VUMETER_H_
#include "fsl_common.h"

#define NSAMPLES 1024
#define NBINS 8

void Vumeter_Init();
void Vumeter_Generate(int16_t * s);
void Vumeter_Display();
void Vumeter_Clear();
bool Vumeter_BackBufferEmpty();

#endif /* VUMETER_H_ */
