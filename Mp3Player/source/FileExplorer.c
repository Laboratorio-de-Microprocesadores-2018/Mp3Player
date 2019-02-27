/*
 * FileExplorer.c
 */

#include "FileExplorer.h"
/*
#include "fsl_sysmpu.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_sd.h"
#include "fsl_sd_disk.h"
#include "fsl_debug_console.h"*/
#include <stdio.h>
#include <string.h>
#include "fsl_sd.h"
#include "fsl_debug_console.h"
#include "ff.h"
#include "diskio.h"
#include "fsl_sd_disk.h"
#include "board.h"

#include "fsl_sysmpu.h"
#include "pin_mux.h"
#include "clock_config.h"

static status_t sdcardWaitCardInsert(void);


/*! @brief SDMMC host detect card configuration */
static const sdmmchost_detect_card_t s_sdCardDetect = {
#ifndef BOARD_SD_DETECT_TYPE
    .cdType = kSDMMCHOST_DetectCardByGpioCD,
#else
    .cdType = BOARD_SD_DETECT_TYPE,
#endif
    .cdTimeOut_ms = (~0U),
};




status_t FE_Init()
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
	 PORT_SetPinConfig(PORTE, 2, &porte_pin_config);	  /* PORTE2 (pin 3) is configured as SDHC0_DCLK */
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

	 SYSMPU_Enable(SYSMPU, false);
}

bool FE_check4Drive()
{
    if (sdcardWaitCardInsert() == kStatus_Success)//CAMBIAR! es bloqueante.
        return true;
    else
    	return false;
}




static status_t sdcardWaitCardInsert(void)
{
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
        PRINTF("\r\nSD host init fail\r\n");
        return kStatus_Fail;
    }
    /* power off card */
    SD_PowerOffCard(g_sd.host.base, g_sd.usrParam.pwr);
    /* wait card insert */
    if (SD_WaitCardDetectStatus(SD_HOST_BASEADDR, &s_sdCardDetect, true) == kStatus_Success)
    {
        PRINTF("\r\nCard inserted.\r\n");
        /* power on the card */
        SD_PowerOnCard(g_sd.host.base, g_sd.usrParam.pwr);
    }
    else
    {
        //PRINTF("\r\nCard detect fail.\r\n");
        return kStatus_Fail;
    }

    return kStatus_Success;
}

