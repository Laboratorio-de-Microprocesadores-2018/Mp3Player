/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ffconf.h"
/* This fatfs subcomponent is disabled by default
 * To enable it, define following macro in ffconf.h */
#ifdef SD_DISK_ENABLE

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "fsl_sd_disk.h"

#include "fsl_sysmpu.h"
#include "pin_mux.h"

/*******************************************************************************
 * Definitons
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Card descriptor */
sd_card_t g_sd;

/*! @brief SDMMC host detect card configuration */
static const sdmmchost_detect_card_t s_sdCardDetect = {
#ifndef BOARD_SD_DETECT_TYPE
    .cdType = kSDMMCHOST_DetectCardByGpioCD,
#else
    .cdType = BOARD_SD_DETECT_TYPE,
#endif
    .cdTimeOut_ms = (0U),//Checks only one time
};



/*******************************************************************************
 * Code
 ******************************************************************************/
DRESULT sd_disk_write(uint8_t physicalDrive, const uint8_t *buffer, uint32_t sector, uint8_t count)
{
    if (physicalDrive != SDDISK)
    {
        return RES_PARERR;
    }

    if (kStatus_Success != SD_WriteBlocks(&g_sd, buffer, sector, count))
    {
        return RES_ERROR;
    }

    return RES_OK;
}

DRESULT sd_disk_read(uint8_t physicalDrive, uint8_t *buffer, uint32_t sector, uint8_t count)
{
    if (physicalDrive != SDDISK)
    {
        return RES_PARERR;
    }

    if (kStatus_Success != SD_ReadBlocks(&g_sd, buffer, sector, count))
    {
        return RES_ERROR;
    }

    return RES_OK;
}

DRESULT sd_disk_ioctl(uint8_t physicalDrive, uint8_t command, void *buffer)
{
    DRESULT result = RES_OK;

    if (physicalDrive != SDDISK)
    {
        return RES_PARERR;
    }

    switch (command)
    {
        case GET_SECTOR_COUNT:
            if (buffer)
            {
                *(uint32_t *)buffer = g_sd.blockCount;
            }
            else
            {
                result = RES_PARERR;
            }
            break;
        case GET_SECTOR_SIZE:
            if (buffer)
            {
                *(uint32_t *)buffer = g_sd.blockSize;
            }
            else
            {
                result = RES_PARERR;
            }
            break;
        case GET_BLOCK_SIZE:
            if (buffer)
            {
                *(uint32_t *)buffer = g_sd.csd.eraseSectorSize;
            }
            else
            {
                result = RES_PARERR;
            }
            break;
        case CTRL_SYNC:
            result = RES_OK;
            break;
        default:
            result = RES_PARERR;
            break;
    }

    return result;
}

DSTATUS sd_disk_status(uint8_t physicalDrive)
{
	if (physicalDrive != SDDISK)
	{
		return STA_NOINIT;
	}

	if(SD_IsCardPresent(0)==false)
		return STA_NODISK;
	else
		return 0;

}

DSTATUS sd_disk_initialize(uint8_t physicalDrive)
{
    if (physicalDrive != SDDISK)
    {
        return STA_NOINIT;
    }

    if(g_sd.isHostReady)
    {
        /* reset host */
        SD_HostReset(&(g_sd.host));
    }
    else
    {
        return STA_NOINIT;
    }

    if (kStatus_Success != SD_CardInit(&g_sd))
    {
        SD_CardDeinit(&g_sd);
        memset(&g_sd, 0U, sizeof(g_sd));
        return STA_NOINIT;
    }

    return 0;
}

status_t sd_disk_setUp(void)
{
	CLOCK_EnableClock(kCLOCK_PortE);

	 const port_pin_config_t porte_pin_config = {
		kPORT_PullUp,                                            /* Internal pull-up resistor is enabled */
		kPORT_FastSlewRate,                                      /* Fast slew rate is configured */
		kPORT_PassiveFilterDisable,                              /* Passive filter is disabled */
		kPORT_OpenDrainDisable,                                  /* Open drain is disabled */
		kPORT_HighDriveStrength,                                 /* High drive strength is configured */
		kPORT_MuxAlt4,                                           /* Pin is configured as SDHC0_D1 */
		kPORT_UnlockRegister                                     /* Pin Control Register fields [15:0] are not locked */
	  };

	 PORT_SetPinConfig(PORTE, 0, &porte_pin_config);   /* PORTE0 (pin 1) is configured as SDHC0_D1 */
	 PORT_SetPinConfig(PORTE, 1, &porte_pin_config);   /* PORTE1 (pin 2) is configured as SDHC0_D0 */
	 PORT_SetPinConfig(PORTE, 2, &porte_pin_config);	/* PORTE2 (pin 3) is configured as SDHC0_DCLK */
	 PORT_SetPinConfig(PORTE, 3, &porte_pin_config);   /* PORTE3 (pin 4) is configured as SDHC0_CMD */
	 PORT_SetPinConfig(PORTE, 4, &porte_pin_config);   /* PORTE4 (pin 5) is configured as SDHC0_D3 */
	 PORT_SetPinConfig(PORTE, 5, &porte_pin_config);   /* PORTE5 (pin 6) is configured as SDHC0_D2 */



	  const port_pin_config_t porte6_pin7_config = {
		kPORT_PullDown,                                          /* Internal pull-down resistor is enabled */
		kPORT_FastSlewRate,                                      /* Fast slew rate is configured */
		kPORT_PassiveFilterDisable,                              /* Passive filter is disabled */
		kPORT_OpenDrainDisable,                                  /* Open drain is disabled */
		kPORT_LowDriveStrength,                                  /* Low drive strength is configured */
		kPORT_MuxAsGpio,                                         /* Pin is configured as PTE6 */
		kPORT_UnlockRegister                                     /* Pin Control Register fields [15:0] are not locked */
	  };
	 PORT_SetPinConfig(PORTE, 6, &porte6_pin7_config);   /* PORTE6 (pin 7) is configured as PTE6 */

	 /*-------------------------------SD PINS END----------------------------------*/
	 SYSMPU_Enable(SYSMPU, false);

	 /* Save host information. */
	g_sd.host.base = SD_HOST_BASEADDR;
	g_sd.host.sourceClock_Hz = SD_HOST_CLK_FREQ;
	/* card detect type */
	g_sd.usrParam.cd = &s_sdCardDetect;
#if defined DEMO_SDCARD_POWER_CTRL_FUNCTION_EXIST
	g_sd.usrParam.pwr = &s_sdCardPwrCtrl;
#endif
	/* SD host init function */
	if (SD_HostInit(&g_sd) != kStatus_Success)
	{
		//PRINTF("\r\nSD host init fail\r\n");
		return kStatus_Fail;
	}



}

#endif /* SD_DISK_ENABLE */
