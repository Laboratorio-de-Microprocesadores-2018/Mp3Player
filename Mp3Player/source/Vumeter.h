#ifndef VUMETER_H_
#define VUMETER_H_
#include "stdint.h"

#define NSAMPLES 1024
#define NBINS 8

void Vumeter_Init();
void Vumeter_Generate(int16_t * s);
void Vumeter_Display();
void Vumeter_Clear();

#endif /* VUMETER_H_ */
