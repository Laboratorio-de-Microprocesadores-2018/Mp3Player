/**
 * @file LM49450.c
 * @brief
 *
 *
 */


#include "LM49450.h"
#include "fsl_i2c.h"
#include "fsl_port.h"
#include "fsl_debug_console.h"

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

#define LM49450_I2C_MASTER_SLAVE_ADDR 	0b1111101
#define LM49450_I2C_BAUDRATE 			100000U

// Store the status of the registers, init with default values
static LM49450_Registers registers = {	.CTRL 		 = 0x01,
										.CLK 		 = 0x00,
										.CHRGPMP_FRQ = 0x49,
										.I2S_MODE 	 = 0x00,
										.I2S_CLK 	 = 0x00,
										.HP_3D_CTRL  = 0x00,
										.SPK_3D_CTRL = 0x00,
										.HP_VOLUME 	 = 0x14,
										.SPK_VOLUME  = 0x10,
										.CMP_0_LSB 	 = 0x0F,
										.CMP_0_MSB 	 = 0xFF,
										.CMP_1_LSB 	 = 0xFF,
										.CMP_1_MSB 	 = 0x00,
										.CMP_2_LSB 	 = 0xFF,
										.CMP_2_MSB 	 = 0x00};


static void LM49450_WriteReg(uint8_t regAddress, uint8_t data)
{
	// I2C transfer configuration
	static i2c_master_transfer_t transfer;

	transfer.slaveAddress 	 = LM49450_I2C_MASTER_SLAVE_ADDR,
	transfer.direction 	 	 = kI2C_Write,
	transfer.subaddressSize  = 1,
	transfer.dataSize 		 = 1,
	transfer.flags 		 	 = kI2C_TransferDefaultFlag;
    transfer.subaddress 	 = regAddress;
    transfer.data  			 = &data;
    I2C_MasterTransferBlocking(LM49450_I2C , &transfer);
}


static void LM49450_ReadReg(uint8_t regAddress, uint8_t* data)
{
	// I2C transfer configuration
	static i2c_master_transfer_t transfer;

	transfer.slaveAddress 	 = LM49450_I2C_MASTER_SLAVE_ADDR,
	transfer.direction 	 	 = kI2C_Read,
	transfer.subaddressSize  = 1,
	transfer.dataSize 		 = 1,
	transfer.flags 		 	 = kI2C_TransferDefaultFlag;
    transfer.subaddress 	 = regAddress;
    transfer.data  			 = data;
    I2C_MasterTransferBlocking(LM49450_I2C , &transfer);
}
/**
 *
 */
void LM49450_GetDefaultSlaveConfig(LM49450_SlaveConfig * config)
{
	config->reference = LM49450_InternalRef;
	config->oversampleRate = LM49450_DAC_OSR_125;
	config->defaultDacFilter = true;
	config->oscillatorMode = LM49450_FixedFrequency;
	config->mute = false;
	config->lineInEnable = false;
	config->enable = true;
	config->dither = LM49450_DitherDefault;
	config->MclkDiv = 0x00;
	config->chargePumpDiv = 0x49;
	config->wordSize = LM49450_I2sWordSize_16;
	config->stereoMode = LM49450_StereoNormal;
	config->wordOrder = LM49450_WordOrderNormal;
	config->I2sMode = LM49450_I2s_Normal;
	config->I2sClkDiv = 0x00;
	config->bitsPerWord = LM49450_I2sBitsPerWord_16;
	config->wordSelectLineMaster = false;
	config->clockLineMaster = false;
	config->headphone3D.enable = false;
	config->speaker3D.enable = false;

}

/**
 *
 */
void LM49450_SlaveInit(LM49450_SlaveConfig * config)
{
//	// Init I2C pins
//	port_pin_config_t pinConfig = {
//	kPORT_PullUp,                                            /* Internal pull-up resistor is enabled */
//	kPORT_FastSlewRate,                                      /* Fast slew rate is configured */
//	kPORT_PassiveFilterDisable,                              /* Passive filter is disabled */
//	kPORT_OpenDrainEnable,                                   /* Open drain is enabled */
//	kPORT_LowDriveStrength,                                  /* Low drive strength is configured */
//	kPORT_MuxAlt5,                                           /* Pin is configured as I2C0_SCL */
//	kPORT_UnlockRegister                                     /* Pin Control Register fields [15:0] are not locked */
//	};
//	PORT_SetPinConfig(PORTE, 24, &pinConfig); /* PORTE24 (pin 31) is configured as I2C0_SCL */
//	PORT_SetPinConfig(PORTE, 25, &pinConfig); /* PORTE25 (pin 32) is configured as I2C0_SDA */

	// Init I2C module
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

	registers.CTRL = CTRL_EXT_REF(config->reference) 		| \
					 CTRL_DAC_MODE(config->oversampleRate) 	| \
					 CTRL_COMP(!config->defaultDacFilter)	| \
					 CTRL_SS(config->oscillatorMode)		| \
					 CTRL_MUTE(0) 							| \
					 CTRL_LINE_IN(config->lineInEnable) 	| \
					 CTRL_ENABLE(config->enable);

	registers.CLK = CLK_DAC_DITHER(config->dither) | CLK_RDIV(config->MclkDiv);

	registers.CHRGPMP_FRQ = CHRGPMP_FRQ_CPDIV(config->chargePumpDiv);

	registers.I2S_MODE = I2S_MODE_WRD(config->wordSize)				| \
			 			 I2S_MODE_STEREO_REVERSE(config->stereoMode)| \
		 				 I2S_MODE_WORD_ORDER(config->wordOrder)		| \
						 I2S_MODE_MODE(config->I2sMode);

	registers.I2S_CLK = I2S_CLK_CLK(config->I2sClkDiv)				|\
						I2S_CLK_WS(config->bitsPerWord)				|\
						I2S_CLK_WS_MS(config->wordSelectLineMaster)	|\
						I2S_CLK_CLK_MS(config->clockLineMaster);

	LM49450_WriteReg(0,  registers.CTRL);
	LM49450_WriteReg(1,  registers.CLK);
	LM49450_WriteReg(2,  registers.CHRGPMP_FRQ);
	LM49450_WriteReg(3,  registers.I2S_MODE);
	LM49450_WriteReg(4,  registers.I2S_CLK);
	LM49450_WriteReg(5,  registers.HP_3D_CTRL);
	LM49450_WriteReg(6,  registers.SPK_3D_CTRL);
	LM49450_WriteReg(7,  registers.HP_VOLUME);
	LM49450_WriteReg(8,  registers.SPK_VOLUME);
	LM49450_WriteReg(10, registers.CMP_0_LSB);
	LM49450_WriteReg(11, registers.CMP_0_MSB);
	LM49450_WriteReg(12, registers.CMP_1_LSB);
	LM49450_WriteReg(13, registers.CMP_1_MSB);
	LM49450_WriteReg(14, registers.CMP_2_LSB);
	LM49450_WriteReg(15, registers.CMP_2_MSB);

	LM49450_Registers registersCheck;

	LM49450_ReadReg(0,  &registersCheck.CTRL);
	LM49450_ReadReg(1,  &registersCheck.CLK);
	LM49450_ReadReg(2,  &registersCheck.CHRGPMP_FRQ);
	LM49450_ReadReg(3,  &registersCheck.I2S_MODE);
	LM49450_ReadReg(4,  &registersCheck.I2S_CLK);
	LM49450_ReadReg(5,  &registersCheck.HP_3D_CTRL);
	LM49450_ReadReg(6,  &registersCheck.SPK_3D_CTRL);
	LM49450_ReadReg(7,  &registersCheck.HP_VOLUME);
	LM49450_ReadReg(8,  &registersCheck.SPK_VOLUME);
	LM49450_ReadReg(10, &registersCheck.CMP_0_LSB);
	LM49450_ReadReg(11, &registersCheck.CMP_0_MSB);
	LM49450_ReadReg(12, &registersCheck.CMP_1_LSB);
	LM49450_ReadReg(13, &registersCheck.CMP_1_MSB);
	LM49450_ReadReg(14, &registersCheck.CMP_2_LSB);
	LM49450_ReadReg(15, &registersCheck.CMP_2_MSB);

	if(memcmp(&registers,&registersCheck,sizeof(LM49450_Registers))==0)
		PRINTF("LM49450 Config OK! \n");
	else
		PRINTF("LM49450 Config ERROR! \n");


}


bool LM49450_SetSampleRate(int32_t mclk, uint32_t sampleRate)
{


	uint8_t dacMode = (registers.CTRL&CTRL_DAC_MODE_MASK)>>CTRL_DAC_MODE_SHIFT;

	uint32_t OSR[] = {250,256,128,128};

	uint32_t clkB = sampleRate * OSR[dacMode];

	 assert(mclk >= clkB);

	uint32_t rdiv = (mclk<<1)/clkB-1;

	if(rdiv>=0 && rdiv<=63)
	{
		registers.CLK = (registers.CLK & ~CLK_RDIV_MASK) | CLK_RDIV(rdiv);
		LM49450_WriteReg(1,registers.CLK);
		return true;
	}
	else
		return false;

}


/**
 *
 */
void LM49450_Enable(bool enable)
{
	if(enable == true)
		registers.CTRL |= CTRL_ENABLE_MASK;
	else
		registers.CTRL &= ~CTRL_ENABLE_MASK;

	LM49450_WriteReg(0,  registers.CTRL);
}

/**
 *
 */
void LM49450_Mute()
{

}

/**
 *	@brief
 *
 */
void LM49450_SetHpVolume(uint8_t vol)
{
	assert(0<=vol && vol<=32);

	registers.HP_VOLUME = vol;
	LM49450_WriteReg(7,  registers.HP_VOLUME);
}

void LM49450_SetSpkVolume(uint8_t vol)
{
	assert(0<=vol && vol<=32);

	registers.SPK_VOLUME = vol;
	LM49450_WriteReg(8,  registers.SPK_VOLUME);
}

uint8_t LM49450_GetHpVolume(void)
{
	return registers.HP_VOLUME;
}

uint8_t LM49450_GetSpkVolume(void)
{
	return registers.SPK_VOLUME;
}
/**
 *
 */
bool LM49450_VolumeUp() /// CAMBIAR PARA QUE CHEQUEE SI ESTAN LOS HP O NO
					    /// COMO? HABRIA QUE AGREGAR UN GPIO QUE LO SENSE
{
	bool retval = false;

	if(registers.HP_VOLUME < 31)
	{
		registers.HP_VOLUME++;
		LM49450_WriteReg(7,  registers.HP_VOLUME);
		retval = true;
	}
	return retval;
}

/**
 *
 */
bool LM49450_VolumeDown()
{
	bool retval = false;
	if(registers.HP_VOLUME > 0)
	{
		registers.HP_VOLUME--;
		LM49450_WriteReg(7,  registers.HP_VOLUME);
		retval = true;
	}
	return retval;
}

void LM49450_GetDefault3DConfig(LM49450_3Dconfig * config)
{
	config->attenuate6db = false;
	config->hpFreq = highPassFreq_900Hz;
	config->mixLevel = mixLevel_37_pc;
	config->mode = modeWide;
	config->enable = true;
}
void LM49450_Set3DConfig(LM49450_3Dconfig * config)
{
	registers.HP_3D_CTRL = 	HP_3D_CTRL_ATN(config->attenuate6db)	|\
							HP_3D_CTRL_FREQ(config->hpFreq)			|\
							HP_3D_CTRL_GAIN(config->mixLevel)		|\
							HP_3D_CTRL_MODE(config->mode)			|\
							HP_3D_CTRL_EN(config->enable);

	LM49450_WriteReg(5,  registers.HP_3D_CTRL);
}
