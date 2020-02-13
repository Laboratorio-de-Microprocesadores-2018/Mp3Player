/**
 * @file CPUTimeMeasurement.h
 * @brief
 *
 *
 */

#ifndef CPUTIMEMEASUREMENT_H_
#define CPUTIMEMEASUREMENT_H_


#include "pin_mux.h"

#define MEASURE_CPU_TIME

#ifdef MEASURE_CPU_TIME

	#define SET_DBG_PIN(n)  BITBAND_REG(DBG##n##_GPIO->PDOR, DBG##n##_PIN) = 1
	#define CLEAR_DBG_PIN(n) BITBAND_REG(DBG##n##_GPIO->PDOR, DBG##n##_PIN) = 0
	#define TOGGLE_DBG_PIN(n) BITBAND_REG(DBG##n##_GPIO->PTOR, DBG##n##_PIN) = 1

#else
	#define SET_DBG_PIN(n)
	#define CLEAR_DBG_PIN(n)
	#define TOGGLE_DBG_PIN(n)
#endif


#endif /* CPUTIMEMEASUREMENT_H_ */
