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

static bool init = false;
// Static colors

static const int rotation[4] = {0, 1, 3, 2};
static int rotationIndex = 1;

static const Color Green  = {.RGB={0,20,0}};
static const Color Yellow = {.RGB={14,14,0}};
static const Color Orange = {.RGB={18,6,1}};
static const Color Red    = {.RGB={20,0,0}};
static const Color White  = {.RGB={20,20,20}};
static const Color Clear  = {.RGB={0,0,0}};
static const Color Blue   = {.RGB={0,0,40}};

static const Color colorBar[MATRIX_HEIGHT] = {Green,Green,Green,Yellow,Yellow,Orange,Orange,Red};

// Vumeter screen
static Color screen[8][8];

void Vumeter_Init()
{
	//arm_status s = arm_rfft_init_q15(&fftInstance,NSAMPLES,0,1);
	arm_status s = arm_rfft_fast_init_f32(&fftInstance,NSAMPLES);

	LedMatrix_Init();

	// Compute logaritmic spaced intervals
	float step = log10f(NSAMPLES/2*0.5)/(float)NBINS;
	for(int i=1; i<=NBINS; i++)
		binLimits[i]=(pow(10,i*step)+0.5);

	backBufferEmpty = true;

	init = true;

}

void Vumeter_Deinit()
{
	LedMatrix_Deinit();
}

bool Vumeter_BackBufferEmpty()
{
	return backBufferEmpty;
}

void Vumeter_RotateLeft()
{
	rotationIndex = (rotationIndex + 1)%4;
}
void Vumeter_RotateRight()
{
	rotationIndex = (rotationIndex + 4 - 1)%4;
}

// ~760us xxxx Ahora tarda 1.2~1.3ms (Si se modifica la funcion, medir de vuelta!)
void Vumeter_Generate(int16_t * ss)
{
	if(init == true)
	{
		backBufferEmpty = false;

		// Take only one channel samples (skip right channel)
		int16_t s[NSAMPLES];
		for(int i=0; i<NSAMPLES; i++)
			s[i]=ss[2*i]; // TODO: Make compatible with mono!!

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
		uint8_t binValues[NBINS] = {};
		for(int i=0; i<NBINS; i++)
		{
			arm_mean_f32(&samples[binLimits[i]], binLimits[i+1]-binLimits[i], &mean[i]);
			mean[i] = log10f(mean[i]);

			// Convert to an integer to send to bars on the display
			// (constrain 0-NBINS)
			binValues[i] = MIN(MAX((uint8_t)((mean[i]+1)*3), 0), NBINS);
		}


		int8_t rot = rotation[rotationIndex];

		int dirX = rot & 0x02 ? -1 : 1;
		int dirY = rot & 0x01 ? -1 : 1;

		uint8_t offsetX = dirX == 1 ? 0 : MATRIX_HEIGHT-1;
		uint8_t offsetY = dirY == 1 ? 0 : MATRIX_HEIGHT-1;

		bool invXY = __builtin_parity(rot);

		int realX,realY;

		for(int y=0; y<MATRIX_HEIGHT; y++)
		{

			realY = offsetY + dirY * y;

			int x;
			for(x=0; x < binValues[y]; x++)
			{
				Color c = colorBar[x];

				realX = offsetX + dirX * x;

				if(invXY)
					screen[realX][realY] = c;
				else
					screen[realY][realX] = c;

			}
			while(x<MATRIX_WIDTH)
			{
				realX = offsetX + dirX * x;

				if(invXY)
					screen[realX][realY] = Clear;
				else
					screen[realY][realX] = Clear;

				x++;
			}
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
