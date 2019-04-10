/**
 * @file LM49450.h
 * @brief Audio subsystem LM49450 driver.
 * The registers of the chip are written using I2C blocking functions.
 *
 */

/////////////////////////////////////////////////////////////////////////////////
//                             Included header files                           //
/////////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stdint.h>

/////////////////////////////////////////////////////////////////////////////////
//                    Enumerations, structures and typedefs                    //
/////////////////////////////////////////////////////////////////////////////////

/**
 * @brief
 */
typedef enum
{
	LM49450_InternalRef,
	LM49450_ExternalRef
}LM49450_Reference;

/**
 * @brief
 */
typedef enum
{
	LM49450_DAC_OSR_125,
	LM49450_DAC_OSR_128,
	LM49450_DAC_OSR_64,
	LM49450_DAC_OSR_32
}LM49450_OversampleRate;

/**
 * @brief
 */
typedef enum
{
	LM49450_FixedFrequency,
	LM49450_SpreadSpectrum
}LM49450_OscillatorMode;

/**
 * @brief
 */
typedef enum
{
	LM49450_DAC_Mode,
	LM49450_LineIN_Mode
}LM49450_Mode;

/**
 * @brief
 */
typedef enum
{
	LM49450_DitherDefault,
	LM49450_DitherOn,
	LM49450_DitherOff
}LM49450_Dither;

/**
 * @brief
 */
typedef enum
{
	LM49450_I2sWordSize_16,
	LM49450_I2sWordSize_18,
	LM49450_I2sWordSize_20,
	LM49450_I2sWordSize_22,
	LM49450_I2sWordSize_24,
	LM49450_I2sWordSize_25,
	LM49450_I2sWordSize_26,
	LM49450_I2sWordSize_32
}LM49450_I2sWordSize;

/**
 * @brief
 */
typedef enum
{
	LM49450_I2sClkDiv_1 		= 0,
	LM49450_I2sClkDiv_2 		= 1,
	LM49450_I2sClkDiv_4 		= 2,
	LM49450_I2sClkDiv_6 		= 7,
	LM49450_I2sClkDiv_8 		= 0,
	LM49450_I2sClkDiv_10 		= 3,
	LM49450_I2sClkDiv_16 		= 4,
	LM49450_I2sClkDiv_20 		= 7,
	LM49450_I2sClkDiv_2_5 		= 8,
	LM49450_I2sClkDiv_3 		= 9,
	LM49450_I2sClkDiv_3_90625 	= 10 ,
	LM49450_I2sClkDiv_5 		= 11,
	LM49450_I2sClkDiv_7_8125 	= 12,
}LM49450_I2sClkDiv;

/**
 * @brief
 */
typedef enum
{
	LM49450_StereoNormal,
	LM49450_StereoReverse
}LM49450_StereoMode;

/**
 * @brief
 */
typedef enum
{
	LM49450_WordOrderNormal,
	LM49450_WordOrderReverse
}LM49450_WordOrder;

/**
 * @brief
 */
typedef enum
{
	LM49450_I2s_Normal,
	LM49450_I2s_LeftJustified,
	LM49450_I2s_RightJustified
}LM49450_I2sMode;

/**
 * @brief
 */
typedef enum
{
	LM49450_I2sBitsPerWord_16,
	LM49450_I2sBitsPerWord_25,
	LM49450_I2sBitsPerWord_32,
}LM49450_I2sBitsPerWord;

/**
 * @brief
 */
typedef enum
{
	highPassFreq_0Hz,
	highPassFreq_300Hz,
	highPassFreq_600Hz,
	highPassFreq_900Hz
}LM49450_3D_HighPassFreq;

/**
 * @brief
 */
typedef enum
{
	mixLevel_25_pc,
	mixLevel_37_pc,
	mixLevel_50_pc,
	mixLevel_75_pc
}LM49450_3D_MixLevel;

/**
 * @brief
 */
typedef enum
{
	modeNarrow,
	modeWide
}LM49450_3D_Mode;

/**
 * @brief
 */
typedef struct
{
	bool attenuate6db;
	LM49450_3D_HighPassFreq hpFreq;
	LM49450_3D_MixLevel mixLevel;
	LM49450_3D_Mode mode;
	bool enable;
}LM49450_3Dconfig;


/**
 * @brief
 */
typedef struct
{
	LM49450_Reference reference;
	LM49450_OversampleRate oversampleRate;
	bool defaultDacFilter;
	LM49450_OscillatorMode oscillatorMode;
	bool mute;
	LM49450_Mode mode;
	bool enable;
	LM49450_Dither dither;
	uint8_t MclkDiv; // Divided by 2
	uint8_t chargePumpDiv; // Divided by 2
	LM49450_I2sWordSize wordSize;
	LM49450_StereoMode stereoMode;
	LM49450_WordOrder wordOrder;
	LM49450_I2sMode I2sMode;
	LM49450_I2sClkDiv I2sClkDiv;
	LM49450_I2sBitsPerWord bitsPerWord;
	bool wordSelectLineMaster;
	bool clockLineMaster;
	LM49450_3Dconfig headphone3D;
	LM49450_3Dconfig speaker3D;
}LM49450_Config;

/////////////////////////////////////////////////////////////////////////////////
//                         				API	  		                           //
/////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
void LM49450_GetDefaultConfig(LM49450_Config * config);

/**
 *
 */
void LM49450_Init(LM49450_Config * config);

/**
 *
 */
void LM49450_Mute();

/**
 *	@brief Set the operation mode
 *
 *	@param mode Te operation mode
 */
void LM49450_SetMode(LM49450_Mode mode);

/**
 *	@brief Increase the volume of the output.
 *	@return True if applied, false if maximum reached
 */
bool LM49450_VolumeUp();

/**
 *	@brief Decrease the volume of the output.
 *	@return True if applied, false if minimum reached
 */
bool LM49450_VolumeDown();


