#include "Equalizer.h"


#include "arm_math.h"
#include "assert.h"
///#include "math_helper.h"

/* Number of 2nd order Biquad stages per filter */
#define NUMSTAGES 		2U

#define EQ_NUM_BANDS 	5U

/* ----------------------------------------------------------------------
** Q31 state buffers for Band1, Band2, Band3, Band4, Band5
** ------------------------------------------------------------------- */
static q15_t biquadStateBand1[4 * NUMSTAGES];
//static q63_t biquadStateBand2[4 * NUMSTAGES];
//static q31_t biquadStateBand3[4 * NUMSTAGES];
//static q31_t biquadStateBand4[4 * NUMSTAGES];
//static q31_t biquadStateBand5[4 * NUMSTAGES];


//static arm_biquad_cas_df1_32x64_ins_q31 	S1;
static arm_biquad_casd_df1_inst_q15 S1;
//arm_biquad_cas_df1_32x64_ins_q31 	S2;
//arm_biquad_casd_df1_inst_q31 		S3;
//arm_biquad_casd_df1_inst_q31 		S4;
//arm_biquad_casd_df1_inst_q31 		S5;


/* ----------------------------------------------------------------------
** Q31 input and output buffers
** ------------------------------------------------------------------- */
static q15_t inputScaled[2500];
static q15_t output[2500];
static q15_t outputScaled[2500];


static bool enable;
/* ----------------------------------------------------------------------
** Entire coefficient table.  There are 10 coefficients per 4th order Biquad
** cascade filter.  The first 10 coefficients correspond to the -9 dB gain
** setting of band 1; the next 10 coefficient correspond to the -8 dB gain
** setting of band 1; and so on.  There are 10*19=190 coefficients in total
** for band 1 (gains = -9, -8, -7, ..., 9).  After this come the 190 coefficients
** for band 2.
**
** The coefficients are in Q29 format and require a postShift of 2.
** ------------------------------------------------------------------- */
static const int8_t dBmin = -9;

static const int8_t dBmax = 9;

static const int8_t nSteps = 19;
//static const int8_t nStages = 2;

#if 0
const q15_t coeffs[21][6] =
{
	{0x3e78, 0x0000, 0x84ae, 0x3d0e, 0x84ae, 0x3b87},
	{0x3ea3, 0x0000, 0x8470, 0x3d22, 0x8470, 0x3bc5},
	{0x3ecc, 0x0000, 0x8435, 0x3d34, 0x8435, 0x3c00},
	{0x3ef4, 0x0000, 0x83fe, 0x3d43, 0x83fe, 0x3c37},
	{0x3f1b, 0x0000, 0x83c9, 0x3d50, 0x83c9, 0x3c6c},
	{0x3f42, 0x0000, 0x8397, 0x3d5b, 0x8397, 0x3c9e},
	{0x3f69, 0x0000, 0x8368, 0x3d64, 0x8368, 0x3ccd},
	{0x3f8f, 0x0000, 0x833b, 0x3d6b, 0x833b, 0x3cfa},
	{0x3fb5, 0x0000, 0x8311, 0x3d6f, 0x8311, 0x3d24},
	{0x3fda, 0x0000, 0x82e9, 0x3d72, 0x82e9, 0x3d4c},
	{0x4000, 0x0000, 0x82c3, 0x3d72, 0x82c3, 0x3d72},
	{0x4026, 0x0000, 0x829f, 0x3d70, 0x829f, 0x3d96},
	{0x404c, 0x0000, 0x827d, 0x3d6c, 0x827d, 0x3db8},
	{0x4072, 0x0000, 0x825d, 0x3d66, 0x825d, 0x3dd8},
	{0x4099, 0x0000, 0x823f, 0x3d5e, 0x823f, 0x3df6},
	{0x40c0, 0x0000, 0x8222, 0x3d53, 0x8222, 0x3e13},
	{0x40e8, 0x0000, 0x8207, 0x3d47, 0x8207, 0x3e2e},
	{0x4111, 0x0000, 0x81ee, 0x3d38, 0x81ee, 0x3e48},
	{0x413a, 0x0000, 0x81d5, 0x3d26, 0x81d5, 0x3e60},
	{0x4165, 0x0000, 0x81be, 0x3d12, 0x81be, 0x3e77},
	{0x4191, 0x0000, 0x81a9, 0x3cfc, 0x81a9, 0x3e8d}
};
#endif


const q15_t coeffs[19][12] =
{
	{0x44cd, 0x0000, 0x8c5d, 0x3359, 0x74a8, 0xc8f2, 0x4000, 0x0000, 0x845a, 0x3be3, 0x7b95, 0xc3f8},
	{0x443d, 0x0000, 0x8c99, 0x3378, 0x744a, 0xc941, 0x4000, 0x0000, 0x8464, 0x3bdc, 0x7b8e, 0xc402},
	{0x43b0, 0x0000, 0x8cdb, 0x3393, 0x73e9, 0xc993, 0x4000, 0x0000, 0x846c, 0x3bd7, 0x7b88, 0xc40b},
	{0x4324, 0x0000, 0x8d21, 0x33aa, 0x7384, 0xc9e7, 0x4000, 0x0000, 0x8473, 0x3bd2, 0x7b83, 0xc413},
	{0x429b, 0x0000, 0x8d6c, 0x33bd, 0x731b, 0xca3e, 0x4000, 0x0000, 0x847a, 0x3bcf, 0x7b7f, 0xc41b},
	{0x4213, 0x0000, 0x8dbc, 0x33cd, 0x72ae, 0xca98, 0x4000, 0x0000, 0x847f, 0x3bcc, 0x7b7b, 0xc421},
	{0x418c, 0x0000, 0x8e11, 0x33d8, 0x723e, 0xcaf5, 0x4000, 0x0000, 0x8483, 0x3bcb, 0x7b79, 0xc427},
	{0x4107, 0x0000, 0x8e6b, 0x33e0, 0x71c9, 0xcb54, 0x4000, 0x0000, 0x8487, 0x3bcb, 0x7b77, 0xc42c},
	{0x4083, 0x0000, 0x8eca, 0x33e3, 0x7150, 0xcbb7, 0x4000, 0x0000, 0x8489, 0x3bcb, 0x7b76, 0xc430},
	{0x4000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
	{0x3f7e, 0x0000, 0x8f96, 0x33df, 0x7050, 0xcc86, 0x4000, 0x0000, 0x848a, 0x3bd0, 0x7b77, 0xc435},
	{0x3efd, 0x0000, 0x9003, 0x33d7, 0x6fca, 0xccf2, 0x4000, 0x0000, 0x8489, 0x3bd4, 0x7b79, 0xc435},
	{0x3e7d, 0x0000, 0x9074, 0x33cb, 0x6f3e, 0xcd61, 0x4000, 0x0000, 0x8487, 0x3bd9, 0x7b7d, 0xc435},
	{0x3dfe, 0x0000, 0x90ea, 0x33bb, 0x6eae, 0xcdd3, 0x4000, 0x0000, 0x8485, 0x3bdf, 0x7b81, 0xc434},
	{0x3d7f, 0x0000, 0x9165, 0x33a8, 0x6e19, 0xce48, 0x4000, 0x0000, 0x8481, 0x3be5, 0x7b86, 0xc431},
	{0x3d01, 0x0000, 0x91e4, 0x3391, 0x6d7f, 0xcec1, 0x4000, 0x0000, 0x847d, 0x3bed, 0x7b8d, 0xc42e},
	{0x3c84, 0x0000, 0x9267, 0x3377, 0x6ce0, 0xcf3c, 0x4000, 0x0000, 0x8478, 0x3bf5, 0x7b94, 0xc429},
	{0x3c06, 0x0000, 0x92ef, 0x3358, 0x6c3c, 0xcfba, 0x4000, 0x0000, 0x8472, 0x3bfe, 0x7b9c, 0xc424},
	{0x3b89, 0x0000, 0x937b, 0x3337, 0x6b92, 0xd03c, 0x4000, 0x0000, 0x846b, 0x3c08, 0x7ba6, 0xc41d}
};

#if 0
float equalizerCoeffs[13][3][6*3]=
{
	{
		{0.500000,0.000000,-0.965247,0.469577,0.920261,-0.429433,0.500000,0.000000,-0.984162,0.485172,0.983325,-0.483781,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.878360,0.415459,0.749502,-0.320543,0.500000,0.000000,-0.947234,0.456492,0.947655,-0.451878,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.571515,0.308354,0.299778,-0.172340,0.500000,0.000000,-0.821082,0.387271,0.846534,-0.376430,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314}
	},

	{	{0.500000,0.000000,-0.962847,0.467372,0.920261,-0.429433,0.500000,0.000000,-0.983824,0.484788,0.983325,-0.483781,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.871226,0.409777,0.749502,-0.320543,0.500000,0.000000,-0.946523,0.455360,0.947655,-0.451878,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.554683,0.298013,0.299778,-0.172340,0.500000,0.000000,-0.821453,0.384519,0.846534,-0.376430,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314}
	},

	{	{0.500000,0.000000,-0.960255,0.464998,0.920261,-0.429433,0.500000,0.000000,-0.983517,0.484434,0.983325,-0.483781,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.863562,0.403721,0.749502,-0.320543,0.500000,0.000000,-0.945902,0.454312,0.947655,-0.451878,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.536881,0.287313,0.299778,-0.172340,0.500000,0.000000,-0.822064,0.381983,0.846534,-0.376430,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314}
	},

	{	{0.500000,0.000000,-0.957455,0.462441,0.920261,-0.429433,0.500000,0.000000,-0.983247,0.484117,0.983325,-0.483781,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.855329,0.397274,0.749502,-0.320543,0.500000,0.000000,-0.945385,0.453364,0.947655,-0.451878,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.518071,0.276283,0.299778,-0.172340,0.500000,0.000000,-0.822945,0.379703,0.846534,-0.376430,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314}
	},

	{	{0.500000,0.000000,-0.954432,0.459691,0.920261,-0.429433,0.500000,0.000000,-0.983020,0.483842,0.983325,-0.483781,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.846489,0.390421,0.749502,-0.320543,0.500000,0.000000,-0.944988,0.452534,0.947655,-0.451878,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.498214,0.264963,0.299778,-0.172340,0.500000,0.000000,-0.824125,0.377717,0.846534,-0.376430,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314}
	},

	{	{0.500000,0.000000,-0.951169,0.456735,0.920261,-0.429433,0.500000,0.000000,-0.982842,0.483616,0.983325,-0.483781,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.837003,0.383150,0.749502,-0.320543,0.500000,0.000000,-0.944728,0.451838,0.947655,-0.451878,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.477280,0.253401,0.299778,-0.172340,0.500000,0.000000,-0.825631,0.376068,0.846534,-0.376430,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314}
	},

	{	{0.500000,0.000000,-0.947650,0.453561,0.920261,-0.429433,0.500000,0.000000,-0.982719,0.483445,0.983325,-0.483781,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.826832,0.375453,0.749502,-0.320543,0.500000,0.000000,-0.944619,0.451295,0.947655,-0.451878,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.455243,0.241661,0.299778,-0.172340,0.500000,0.000000,-0.827487,0.374792,0.846534,-0.376430,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314}
	},

	{	{0.500000,0.000000,-0.943858,0.450159,0.920261,-0.429433,0.500000,0.000000,-0.982656,0.483334,0.983325,-0.483781,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.815939,0.367327,0.749502,-0.320543,0.500000,0.000000,-0.944674,0.450919,0.947655,-0.451878,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.432086,0.229815,0.299778,-0.172340,0.500000,0.000000,-0.829713,0.373925,0.846534,-0.376430,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314}
	},

	{	{0.500000,0.000000,-0.939776,0.446519,0.920261,-0.429433,0.500000,0.000000,-0.982656,0.483288,0.983325,-0.483781,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.804286,0.358772,0.749502,-0.320543,0.500000,0.000000,-0.944904,0.450723,0.947655,-0.451878,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.407804,0.217950,0.299778,-0.172340,0.500000,0.000000,-0.832321,0.373496,0.846534,-0.376430,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314}
	},

	{	{0.500000,0.000000,-0.935390,0.442632,0.920261,-0.429433,0.500000,0.000000,-0.982723,0.483308,0.983325,-0.483781,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.791842,0.349797,0.749502,-0.320543,0.500000,0.000000,-0.945316,0.450718,0.947655,-0.451878,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.382403,0.206161,0.299778,-0.172340,0.500000,0.000000,-0.835318,0.373526,0.846534,-0.376430,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314}
	},

	{	{0.500000,0.000000,-0.930684,0.438491,0.920261,-0.429433,0.500000,0.000000,-0.982857,0.483398,0.983325,-0.483781,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.778579,0.340417,0.749502,-0.320543,0.500000,0.000000,-0.945914,0.450909,0.947655,-0.451878,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.355904,0.194555,0.299778,-0.172340,0.500000,0.000000,-0.838698,0.374027,0.846534,-0.376430,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314}
	},

	{	{0.500000,0.000000,-0.925645,0.434093,0.920261,-0.429433,0.500000,0.000000,-0.983059,0.483556,0.983325,-0.483781,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.764471,0.330656,0.749502,-0.320543,0.500000,0.000000,-0.946696,0.451297,0.947655,-0.451878,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,-0.328345,0.183244,0.299778,-0.172340,0.500000,0.000000,-0.842445,0.374999,0.846534,-0.376430,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314}
	},

	{	{0.500000,0.000000,0.000000,0.000000,-0.000000,-0.000000,0.500000,0.000000,0.000000,0.000000,-0.000000,-0.000000,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,0.000000,0.000000,-0.000000,-0.000000,0.500000,0.000000,0.000000,0.000000,-0.000000,-0.000000,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314},
		{0.500000,0.000000,0.000000,0.000000,-0.000000,-0.000000,0.500000,0.000000,0.000000,0.000000,-0.000000,-0.000000,0.081832,0.000000,0.163664,0.081832,0.316046,-0.183314}
	}
};
#endif

/* ----------------------------------------------------------------------
** Desired gains, in dB, per band
** ------------------------------------------------------------------- */
int gainDB[EQ_NUM_BANDS] = {0, -3, 6, 4, -6};


void Equalizer_Enable(bool b)
{
	enable = b;
	if(b==false)
	{
		memset(biquadStateBand1,0,4 * NUMSTAGES);
	}
}

bool Equalizer_IsEnable()
{
	return enable;
}

void Equalizer_SetLevel(uint8_t band, int8_t level)
{
	assert(0<=band && band<EQ_NUM_BANDS);
	assert(dBmin<=level && level<=dBmax);

	gainDB[band] = level;

	if(band == 0)
	{
		//arm_float_to_q15(equalizerCoeffs[level - dBmin][0],coeffs,18);
		S1.pCoeffs = coeffs[level - dBmin];
		memset(biquadStateBand1,0,4 * NUMSTAGES);
	}
}

void Equalizer_GetLevelLimits(int8_t *min, int8_t *max)
{
	*min = dBmin;
	*max = dBmax;
}

int8_t Equalizer_GetLevel(uint8_t band)
{
	assert(0<=band && band<EQ_NUM_BANDS);
	return gainDB[band];
}


#if 0
void TestFloatToQ15(void)
{
	// TEST IF MATLAB CONVERTS TO Q15 CORRECTLY
	const float coeffsF[21][6] =
	{
		{0.4880522, 0.0000000, -0.9634530, 0.4770010, -0.9634530, 0.4650532},
		{0.4893362, 0.0000000, -0.9653401, 0.4776073, -0.9653401, 0.4669435},
		{0.4905909, 0.0000000, -0.9671285, 0.4781440, -0.9671285, 0.4687348},
		{0.4918198, 0.0000000, -0.9688229, 0.4786123, -0.9688229, 0.4704321},
		{0.4930266, 0.0000000, -0.9704280, 0.4790133, -0.9704280, 0.4720398},
		{0.4942147, 0.0000000, -0.9719482, 0.4793478, -0.9719482, 0.4735625},
		{0.4953878, 0.0000000, -0.9733877, 0.4796167, -0.9733877, 0.4750045},
		{0.4965493, 0.0000000, -0.9747506, 0.4798203, -0.9747506, 0.4763696},
		{0.4977028, 0.0000000, -0.9760408, 0.4799591, -0.9760408, 0.4776620},
		{0.4988519, 0.0000000, -0.9772620, 0.4800333, -0.9772620, 0.4788852},
		{0.5000000, 0.0000000, -0.9784176, 0.4800428, -0.9784176, 0.4800428},
		{0.5011507, 0.0000000, -0.9795111, 0.4799873, -0.9795111, 0.4811381},
		{0.5023078, 0.0000000, -0.9805458, 0.4798666, -0.9805458, 0.4821744},
		{0.5034747, 0.0000000, -0.9815245, 0.4796801, -0.9815245, 0.4831547},
		{0.5046551, 0.0000000, -0.9824502, 0.4794269, -0.9824502, 0.4840820},
		{0.5058530, 0.0000000, -0.9833258, 0.4791061, -0.9833258, 0.4849591},
		{0.5070721, 0.0000000, -0.9841539, 0.4787164, -0.9841539, 0.4857885},
		{0.5083163, 0.0000000, -0.9849369, 0.4782566, -0.9849369, 0.4865728},
		{0.5095896, 0.0000000, -0.9856772, 0.4777248, -0.9856772, 0.4873144},
		{0.5108962, 0.0000000, -0.9863772, 0.4771193, -0.9863772, 0.4880155},
		{0.5122403, 0.0000000, -0.9870389, 0.4764380, -0.9870389, 0.4886783}
	};


	q15_t fixed[6];

	for(int i=0; i<21; i++)
	{
		arm_float_to_q15(coeffsF[i], fixed, 6);

		printf("\t{");
		for(int j=0; j<=4; j++)
			printf("0x%.4x, ",fixed[j]);

		if(i<20)
			printf("0x%.4x},\n",fixed[5]);
		else
			printf("0x%.4x}\n};\n",fixed[5]);
	}
}
#endif

void Equalizer_Init()
{


	arm_biquad_cascade_df1_init_q15(
				&S1,
				NUMSTAGES,
				coeffs[gainDB[0] - dBmin],
				biquadStateBand1, 1);

	return;

  /* Initialize the state and coefficient buffers for all Biquad sections */
//  arm_biquad_cas_df1_32x64_init_q31(&S1, NUMSTAGES,
//		  (q31_t *) &coeffTable[190*0 + 10*(gainDB[0] + 9)],
//            &biquadStateBand1Q31[0], 2);

//  arm_biquad_cas_df1_32x64_init_q31(&S2, NUMSTAGES,
//            (q31_t *) &coeffTable[190*1 + 10*(gainDB[1] + 9)],
//            &biquadStateBand2Q31[0], 2);
//
//  arm_biquad_cascade_df1_init_q31(&S3, NUMSTAGES,
//          (q31_t *) &coeffTable[190*2 + 10*(gainDB[2] + 9)],
//          &biquadStateBand3Q31[0], 2);
//
//  arm_biquad_cascade_df1_init_q31(&S4, NUMSTAGES,
//          (q31_t *) &coeffTable[190*3 + 10*(gainDB[3] + 9)],
//          &biquadStateBand4Q31[0], 2);
//
//  arm_biquad_cascade_df1_init_q31(&S5, NUMSTAGES,
//          (q31_t *) &coeffTable[190*4 + 10*(gainDB[4] + 9)],
//          &biquadStateBand5Q31[0], 2);
}

void Equalizer_Apply(int16_t * inputQ15,uint16_t size)
{
	if(enable==true)
	{
		/* ----------------------------------------------------------------------
		** Scale down by 1/4.  This provides additional headroom so that the
		** graphic EQ can apply gain.
		** ------------------------------------------------------------------- */
		arm_scale_q15(inputQ15, 0x7FFF, -2, inputQ15, size);

		arm_biquad_cascade_df1_q15(&S1, inputQ15, inputQ15, size);
		///memcpy(inputQ15,output,size*sizeof(int16_t));

		/* ----------------------------------------------------------------------
		** Scale back up
		** ------------------------------------------------------------------- */
		arm_scale_q15(inputQ15, 0x7FFF, 2, inputQ15, size);
	}
    return;
}
