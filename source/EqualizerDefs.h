#include <arm_math.h>


#define EQ_NUM_STEPS 		 13U 

#define EQ_NUM_COEFFS 		 12U 

#define EQ_NUM_BANDS 		 3U 

#define EQ_NUM_STAGES 		 2U 

#define EQ_MAX_DB 		 6 

#define EQ_MIN_DB 		 -6 

extern const q15_t eqCoeffs[EQ_NUM_BANDS * EQ_NUM_STEPS * EQ_NUM_COEFFS]; 

static inline q15_t * getCoeffs(uint8_t band, int8_t level)
{
	return &eqCoeffs[EQ_NUM_STEPS*EQ_NUM_COEFFS*band + EQ_NUM_COEFFS*(level - EQ_MIN_DB)];
}
