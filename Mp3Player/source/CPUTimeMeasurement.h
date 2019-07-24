/**
 * @file CPUTimeMeasurement.h
 * @brief
 *
 *
 */

#ifndef CPUTIMEMEASUREMENT_H_
#define CPUTIMEMEASUREMENT_H_

#include "fsl_device_registers.h"


//#define MEASURE_CPU_TIME

#ifdef MEASURE_CPU_TIME
	#define MEASURE_CPU_TIME_PORT PORTC
	#define MEASURE_CPU_TIME_GPIO GPIOC
	#define MEASURE_CPU_TIME_PIN	9
	#define SET_TEST_PIN BITBAND_REG(MEASURE_CPU_TIME_GPIO->PDOR, MEASURE_CPU_TIME_PIN) = 1
	#define CLEAR_TEST_PIN BITBAND_REG(MEASURE_CPU_TIME_GPIO->PDOR, MEASURE_CPU_TIME_PIN) = 0
#else
	#define SET_TEST_PIN
	#define CLEAR_TEST_PIN
#endif


#endif /* CPUTIMEMEASUREMENT_H_ */
