/**
 * @file Vumeter.c
 * @brief
 *
 *
 */

#include "Vumeter.h"
#include "LedMatrix.h"
#define ARM_MATH_CM4
#include "arm_math.h"
#include "fsl_common.h"
#define SAMPLES_PER_BIN ((NSAMPLES/2)/NBINS)


//static arm_rfft_instance_q15 fftInstance;
static arm_rfft_fast_instance_f32 fftInstance;

static uint16_t binLimits[NBINS+1];

static bool backBufferEmpty;

// Static colors

static Color Green  = {.RGB={0,20,0}};
static Color Yellow = {.RGB={14,14,0}};
static Color Orange = {.RGB={18,6,1}};
static Color Red    = {.RGB={20,0,0}};
static Color White  = {.RGB={20,20,20}};
static Color Clear  = {.RGB={0,0,0}};
static Color Blue   = {.RGB={0,0,40}};

// Vumeter screen
static Color screen[8][8];

void Vumeter_Init()
{
	//arm_status s = arm_rfft_init_q15(&fftInstance,NSAMPLES,0,1);
	arm_status s = arm_rfft_fast_init_f32(&fftInstance,NSAMPLES);

	LedMatrix_Init();

	// Compute logaritmic spaced intervals
	float step = log10f(NSAMPLES/2)/(float)NBINS;
	for(int i=1; i<=NBINS; i++)
		binLimits[i]=(pow(10,i*step)+0.5);

	backBufferEmpty = true;


}

bool Vumeter_BackBufferEmpty()
{
	return backBufferEmpty;
}

// ~760us xxxx Ahora tarda 1.2~1.3ms (Si se modifica la funcion, medir de vuelta!)
void Vumeter_Generate(int16_t * ss)
{
	backBufferEmpty = false;

	// Take only one channel samples (skip right channel)
	int16_t s[NSAMPLES];
	for(int i=0; i<NSAMPLES; i++)
		s[i]=ss[2*i];

	// Convert samples to float
	float32_t samples[NSAMPLES];
	arm_q15_to_float(s,samples,NSAMPLES);

	// Buffer to store complex FFT
	float32_t fftComplex[2*NSAMPLES];
	arm_rfft_fast_f32(&fftInstance,samples, fftComplex,0);

	// Compute magnitude
	arm_cmplx_mag_f32(fftComplex,samples,NSAMPLES);

	// Compute mean of FFT magnitude for each bin
	float32_t mean[NBINS] = {};
	for(int i=0; i<NBINS; i++)
	{
		arm_mean_f32(&samples[binLimits[i]], binLimits[i+1]-binLimits[i], &mean[i]);
		mean[i] = log10f(mean[i]);
	}

	// Find its maximum
//	float32_t max;
//	uint32_t index;
//	arm_max_f32(mean, NBINS, &max, &index);

	// Convert to an integer to send to bars on the display
	// (constrain 0-NBINS)
	uint8_t binValues[NBINS] = {};
	for(int i=0; i<NBINS; i++)
		binValues[i] = MIN(MAX((uint8_t)((mean[i]+1)/2.5*NBINS), 0), NBINS);


	uint8_t offsetX =  MATRIX_HEIGHT-1;
	int8_t dirX = -1;

	uint8_t offsetY = MATRIX_HEIGHT-1;
	int8_t dirY = -1;

	for(int y=0; y<MATRIX_HEIGHT; y++)
	{
		int x;
		for(x=0; x < binValues[y]; x++)
		{
			if (x < 3)
				screen[offsetY + dirY * y][offsetX + dirX * x] = Green;
			else if (3 <= x && x < 5)
				screen[offsetY + dirY * y][offsetX + dirX * x] = Yellow;
			else if (5 <= x && x < 7)
				screen[offsetY + dirY * y][offsetX + dirX * x] = Orange;
			else if (x == 7)
				screen[offsetY + dirY * y][offsetX + dirX * x] = Red;
		}
		while(x<MATRIX_WIDTH)
		{
			screen[offsetY + dirY * y][offsetX + dirX * x] = Clear;
			x++;
		}
	}



}
// ~190us
void Vumeter_Display()
{
	backBufferEmpty = true;

	LedMatrix_PrintScreen((Color*)screen);
	//LedMatrix_PlainColor(Blue);
}

void Vumeter_Clear()
{
	for(int y=0; y<MATRIX_HEIGHT; y++)
		for(int x=0; x < MATRIX_HEIGHT; x++)
			screen[x][y] = Clear;
	backBufferEmpty = true;

	LedMatrix_Clear();
}
