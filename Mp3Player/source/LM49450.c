/**
 * @file LM49450.c
 * @brief
 *
 *
 */


#include "LM49450.h"
#include "fsl_i2c.h"

#define CTRL_EXT_REF_MASK				(0x80U)
#define CTRL_EXT_REF_SHIFT				(0x07U)
#define CTRL_EXT_REF(x)					(((uint8_t)(((uint8_t)(x)) << CTRL_EXT_REF_SHIFT)) & CTRL_EXT_REF_MASK)

#define CTRL_DAC_MODE_MASK				(0x60U)
#define CTRL_DAC_MODE_SHIFT				(0x05U)
#define CTRL_DAC_MODE(x)				(((uint8_t)(((uint8_t)(x)) << CTRL_DAC_MODE_SHIFT)) & CTRL_DAC_MODE_MASK)

#define CTRL_COMP_MASK					(0x10U)
#define CTRL_COMP_SHIFT					(0x04U)
#define CTRL_COMP(x)					(((uint8_t)(((uint8_t)(x)) << CTRL_COMP_SHIFT)) & CTRL_COMP_MASK)

#define CTRL_SS_MASK					(0x08U)
#define CTRL_SS_SHIFT					(0x03U)
#define CTRL_SS(x)						(((uint8_t)(((uint8_t)(x)) << CTRL_SS_SHIFT)) & CTRL_SS_MASK)

#define CTRL_MUTE_MASK					(0x04U)
#define CTRL_MUTE_SHIFT					(0x02U)
#define CTRL_MUTE(x)					(((uint8_t)(((uint8_t)(x)) << CTRL_MUTE_SHIFT)) & CTRL_MUTE_MASK)

#define CTRL_LINE_IN_MASK				(0x02U)
#define CTRL_LINE_IN_SHIFT				(0x01U)
#define CTRL_LINE_IN(x)					(((uint8_t)(((uint8_t)(x)) << CTRL_LINE_IN_SHIFT)) & CTRL_LINE_IN_MASK)

#define CTRL_ENABLE_MASK				(0x01U)
#define CTRL_ENABLE_SHIFT				(0x00U)
#define CTRL_ENABLE(x)					(((uint8_t)(((uint8_t)(x)) << CTRL_ENABLE_SHIFT)) & CTRL_ENABLE_MASK)



#define CLK_DAC_DITHER_MASK				(0xC0U)
#define CLK_DAC_DITHER_SHIFT			(0x06U)
#define CLK_DAC_DITHER(x)				(((uint8_t)(((uint8_t)(x)) <<  CLK_DAC_DITHER_SHIFT)) & CLK_DAC_DITHER_MASK)

#define CLK_RDIV_MASK					(0x3FU)
#define CLK_RDIV_SHIFT					(0x00U)
#define CLK_RDIV(x)						(((uint8_t)(((uint8_t)(x)) << CLK_RDIV_SHIFT)) & CLK_RDIV_MASK)



#define CHRGPMP_FRQ_CPDIV_MASK			(0xFFU)
#define CHRGPMP_FRQ_CPDIV_SHIFT			(0x00U)
#define CHRGPMP_FRQ_CPDIV(x)			(((uint8_t)(((uint8_t)(x)) << CHRGPMP_FRQ_CPDIV_SHIFT)) & CHRGPMP_FRQ_CPDIV_MASK)



#define I2S_MODE_WRD_MASK				(0x70U)
#define I2S_MODE_WRD_SHIFT				(0x04U)
#define I2S_MODE_WRD(x)					(((uint8_t)(((uint8_t)(x)) << I2S_MODE_WRD_SHIFT)) & I2S_MODE_WRD_MASK)

#define I2S_MODE_STEREO_REVERSE_MASK	(0x08U)
#define I2S_MODE_STEREO_REVERSE_SHIFT	(0x03U)
#define I2S_MODE_STEREO_REVERSE(x)		(((uint8_t)(((uint8_t)(x)) << I2S_MODE_STEREO_REVERSE_SHIFT)) & I2S_MODE_STEREO_REVERSE_MASK)

#define I2S_MODE_WORD_ORDER_MASK		(0x04U)
#define I2S_MODE_WORD_ORDER_SHIFT		(0x02U)
#define I2S_MODE_WORD_ORDER(x)			(((uint8_t)(((uint8_t)(x)) << I2S_MODE_WORD_ORDER_SHIFT)) & I2S_MODE_WORD_ORDER_MASK)

#define I2S_MODE_MODE_MASK				(0x03U)
#define I2S_MODE_MODE_SHIFT				(0x00U)
#define I2S_MODE_MODE(x)				(((uint8_t)(((uint8_t)(x)) << I2S_MODE_MODE_SHIFT)) & I2S_MODE_MODE_MASK)



#define I2S_CLK_CLK_MASK				(0xF0U)
#define I2S_CLK_CLK_SHIFT				(0x04U)
#define I2S_CLK_CLK(x)					(((uint8_t)(((uint8_t)(x)) << I2S_CLK_CLK_SHIFT)) & I2S_CLK_CLK_MASK)

#define I2S_CLK_WS_MASK					(0x0CU)
#define I2S_CLK_WS_SHIFT				(0x02U)
#define I2S_CLK_WS(x)					(((uint8_t)(((uint8_t)(x)) << I2S_CLK_WS_SHIFT)) & I2S_CLK_WS_MASK)

#define I2S_CLK_WS_MS_MASK				(0x02U)
#define I2S_CLK_WS_MS_SHIFT				(0x01U)
#define I2S_CLK_WS_MS(x)				(((uint8_t)(((uint8_t)(x)) << I2S_CLK_WS_MS_SHIFT)) & I2S_CLK_WS_MS_MASK)

#define I2S_CLK_CLK_MS_MASK				(0x01U)
#define I2S_CLK_CLK_MS_SHIFT			(0x00U)
#define I2S_CLK_CLK_MS(x)				(((uint8_t)(((uint8_t)(x)) << I2S_CLK_CLK_MS_SHIFT)) & I2S_CLK_CLK_MS_MASK)



#define HP_3D_CTRL_ATN_MASK				(0x40U)
#define HP_3D_CTRL_ATN_SHIFT			(0x06U)
#define HP_3D_CTRL_ATN(x)				(((uint8_t)(((uint8_t)(x)) << HP_3D_CTRL_ATN_SHIFT)) & HP_3D_CTRL_ATN_MASK)

#define HP_3D_CTRL_FREQ_MASK			(0x30U)
#define HP_3D_CTRL_FREQ_SHIFT			(0x04U)
#define HP_3D_CTRL_FREQ(x)				(((uint8_t)(((uint8_t)(x)) << HP_3D_CTRL_FREQ_SHIFT)) & HP_3D_CTRL_FREQ_MASK)

#define HP_3D_CTRL_GAIN_MASK			(0x0CU)
#define HP_3D_CTRL_GAIN_SHIFT			(0x02U)
#define HP_3D_CTRL_GAIN(x)				(((uint8_t)(((uint8_t)(x)) << HP_3D_CTRL_GAIN_SHIFT)) & HP_3D_CTRL_GAIN_MASK)

#define HP_3D_CTRL_MODE_MASK			(0x02U)
#define HP_3D_CTRL_MODE_SHIFT			(0x01U)
#define HP_3D_CTRL_MODE(x)				(((uint8_t)(((uint8_t)(x)) << HP_3D_CTRL_MODE_SHIFT)) & HP_3D_CTRL_MODE_MASK)

#define HP_3D_CTRL_EN_MASK				(0x01U)
#define HP_3D_CTRL_EN_SHIFT				(0x00U)
#define HP_3D_CTRL_EN(x)				(((uint8_t)(((uint8_t)(x)) << HP_3D_CTRL_EN_SHIFT)) & HP_3D_CTRL_EN_MASK)



#define SPK_3D_CTRL_ATN_MASK			(0x40U)
#define SPK_3D_CTRL_ATN_SHIFT			(0x06U)
#define SPK_3D_CTRL_ATN(x)				(((uint8_t)(((uint8_t)(x)) << SPK_3D_CTRL_ATN_SHIFT)) & SPK_3D_CTRL_ATN_MASK)

#define SPK_3D_CTRL_FREQ_MASK			(0x30U)
#define SPK_3D_CTRL_FREQ_SHIFT			(0x04U)
#define SPK_3D_CTRL_FREQ(x)				(((uint8_t)(((uint8_t)(x)) << SPK_3D_CTRL_FREQ_SHIFT)) & SPK_3D_CTRL_FREQ_MASK)

#define SPK_3D_CTRL_GAIN_MASK			(0x0CU)
#define SPK_3D_CTRL_GAIN_SHIFT			(0x02U)
#define SPK_3D_CTRL_GAIN(x)				(((uint8_t)(((uint8_t)(x)) << SPK_3D_CTRL_GAIN_SHIFT)) & SPK_3D_CTRL_GAIN_MASK)

#define SPK_3D_CTRL_MODE_MASK			(0x02U)
#define SPK_3D_CTRL_MODE_SHIFT			(0x01U)
#define SPK_3D_CTRL_MODE(x)				(((uint8_t)(((uint8_t)(x)) << SPK_3D_CTRL_MODE_SHIFT)) & SPK_3D_CTRL_MODE_MASK)

#define SPK_3D_CTRL_EN_MASK				(0x01U)
#define SPK_3D_CTRL_EN_SHIFT			(0x00U)
#define SPK_3D_CTRL_EN(x)				(((uint8_t)(((uint8_t)(x)) << SPK_3D_CTRL_EN_SHIFT)) & SPK_3D_CTRL_EN_MASK)


#define HP_VOLUME_VOL_MASK				(0x1FU)
#define HP_VOLUME_VOL_SHIFT				(0x00U)
#define HP_VOLUME_VOL(x)				(((uint8_t)(((uint8_t)(x)) << HP_VOLUME_VOL_SHIFT)) & HP_VOLUME_VOL_MASK)



#define SPK_VOLUME_VOL_MASK				(0x1FU)
#define SPK_VOLUME_VOL_SHIFT			(0x00U)
#define SPK_VOLUME_VOL(x)				(((uint8_t)(((uint8_t)(x)) << SPK_VOLUME_VOL_SHIFT)) & SPK_VOLUME_VOL_MASK)



#define CMP_0_LSB_MASK					(0xFFU)
#define CMP_0_LSB_SHIFT  				(0x0U)
#define CMP_0_LSB(x) 					(((uint8_t)(((uint8_t)(x)) << CMP_0_LSB_SHIFT)) & CMP_0_LSB_MASK)



#define CMP_0_MSB_MASK					(0xFFU)
#define CMP_0_MSB_SHIFT  				(0x0U)
#define CMP_0_MSB(x) 					(((uint8_t)(((uint8_t)(x)) << CMP_0_MSB_SHIFT)) & CMP_0_MSB_MASK)



#define CMP_1_LSB_MASK					(0xFFU)
#define CMP_1_LSB_SHIFT  				(0x0U)
#define CMP_1_LSB(x) 					(((uint8_t)(((uint8_t)(x)) << CMP_1_LSB_SHIFT)) & CMP_1_LSB_MASK)



#define CMP_1_MSB_MASK					(0xFFU)
#define CMP_1_MSB_SHIFT  				(0x0U)
#define CMP_1_MSB(x) 					(((uint8_t)(((uint8_t)(x)) << CMP_1_MSB_SHIFT)) & CMP_1_MSB_MASK)



#define CMP_2_LSB_MASK					(0xFFU)
#define CMP_2_LSB_SHIFT  				(0x0U)
#define CMP_2_LSB(x) 					(((uint8_t)(((uint8_t)(x)) << CMP_2_LSB_SHIFT)) & CMP_2_LSB_MASK)



#define CMP_2_MSB_MASK					(0xFFU)
#define CMP_2_MSB_SHIFT  				(0x0U)
#define CMP_2_MSB(x) 					(((uint8_t)(((uint8_t)(x)) << CMP_2_MSB_SHIFT)) & CMP_2_MSB_MASK)





typedef struct
{
	uint8_t CTRL;
	uint8_t CLK;
	uint8_t CHRGPMP_FRQ;
	uint8_t I2S_MODE;
	uint8_t I2S_CLK;
	uint8_t HP_3D_CTRL;
	uint8_t SPK_3D_CTRL;
	uint8_t HP_VOLUME;
	uint8_t SPK_VOLUME;
	uint8_t CMP_0_LSB;
	uint8_t CMP_0_MSB;
	uint8_t CMP_1_LSB;
	uint8_t CMP_1_MSB;
	uint8_t CMP_2_LSB;
	uint8_t CMP_2_MSB;
}LM49450_Registers;


#define LM49450_I2C_MASTER_CLK_SRC 		I2C0_CLK_SRC
#define I2C_MASTER_CLK_FREQ 			CLOCK_GetFreq(I2C0_CLK_SRC)
#define LM49450_I2C 					I2C0

#define LM49450_I2C_MASTER_SLAVE_ADDR 	b1111101
#define LM49450_I2C_BAUDRATE 			100000U

// Store the status of the registers
static LM49450_Registers registers;





/**
 *
 */
void LM49450_GetDefaultConfig(LM49450_Config * config)
{
	config->reference = LM49450_InternalRef;
	config->oversampleRate = LM49450_DAC_OSR_125;
	config->defaultDacFilter = true;
	config->oscillatorMode = LM49450_FixedFrequency;
	config->mute = false;
	config->mode = LM49450_DAC_Mode;
	config->enable = true;
	config->dither = LM49450_DitherDefault;
	config->MclkDiv; /////////////////////////////////////////////////
	config->chargePumpDiv; ////////////////////////////////////////////
	config->wordSize = LM49450_I2sWordSize_16;
	config->stereoMode = LM49450_StereoNormal;
	config->wordOrder = LM49450_WordOrderNormal;
	config->I2sMode = LM49450_I2s_Normal;
	config->I2sClkDiv;/////////////////////////////////////////////////////
	config->bitsPerWord = LM49450_I2sBitsPerWord_16;
	config->wordSelectLineMaster = false;
	config->clockLineMaster = false;
	config->headphone3D.enable = false;
	config->speaker3D.enable = false;

}LM49450_Config;

/**
 *
 */
void LM49450_Init(LM49450_Config * config)
{
	/*
	 * masterConfig->baudRate_Bps = 100000U;
	 * masterConfig->enableStopHold = false;
	 * masterConfig->glitchFilterWidth = 0U;
	 * masterConfig->enableMaster = true;
	 */
	i2c_master_config_t masterConfig;
	I2C_MasterGetDefaultConfig(&masterConfig);
	masterConfig.baudRate_Bps = LM49450_I2C_BAUDRATE;

	I2C_MasterInit(LM49450_I2C, &masterConfig, I2C_MASTER_CLK_FREQ);
}

/**
 *
 */
void LM49450_Mute()
{

}

/**
 *
 */
void LM49450_SetMode(LM49450_Mode mode)
{

}

/**
 *
 */
bool LM49450_VolumeUp()
{

}

/**
 *
 */
bool LM49450_VolumeDown()
{

}



//	 I2C_MasterTransferBlocking(LM49450_I2C, &masterXfer);
//	  masterXfer.slaveAddress = LM49450_I2C_MASTER_SLAVE_ADDR;
//	    masterXfer.direction = kI2C_Read;
//	    masterXfer.subaddress = (uint32_t)deviceAddress;
//	    masterXfer.subaddressSize = 1;
//	    masterXfer.data = g_master_rxBuff;
//	    masterXfer.dataSize = LM49450_I2C_DATA_LENGTH - 1;
//	    masterXfer.flags = kI2C_TransferDefaultFlag;;

