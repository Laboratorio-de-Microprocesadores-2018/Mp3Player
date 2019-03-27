#include "FileExplorer.h"
#include "stdbool.h"

#include "ff.h"
#include "fsl_sysmpu.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_sd.h"
#include "fsl_sd_disk.h"
#include "fsl_debug_console.h"

//static FATFS g_fileSystem; /* File system object */
static FATFS g_fileSystems[3]; /*File system objects for RAM, SD, USB,...*/


/*! @brief SDMMC host detect card configuration */
static const sdmmchost_detect_card_t s_sdCardDetect = {
#ifndef BOARD_SD_DETECT_TYPE
    .cdType = kSDMMCHOST_DetectCardByGpioCD,
#else
    .cdType = BOARD_SD_DETECT_TYPE,
#endif
    .cdTimeOut_ms = (0U),//Checks only one time
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

	    g_fileSystems[SDDISK].pdrv=SDDISK;
	   //g_fileSystems[USBDISK].pdrv=USBDISK;//PONERLO CUANDO SE USE USB
}

status_t FE_check4Drive()
{
//
//	 /* Save host information. */
//	    g_sd.host.base = SD_HOST_BASEADDR;
//	    g_sd.host.sourceClock_Hz = SD_HOST_CLK_FREQ;
//	    /* card detect type */
//	    g_sd.usrParam.cd = &s_sdCardDetect;
//	#if defined DEMO_SDCARD_POWER_CTRL_FUNCTION_EXIST
//	    g_sd.usrParam.pwr = &s_sdCardPwrCtrl;
//	#endif
//	    /* SD host init function */
//	    if (SD_HostInit(&g_sd) != kStatus_Success)
//	    {
//	        PRINTF("\r\nSD host init fail\r\n");
//	        return kStatus_Fail;
//	    }
//
//	    /* power off card */
//	    SD_PowerOffCard(g_sd.host.base, g_sd.usrParam.pwr);
//
//
//	    /* wait card insert */
//	    if (SD_WaitCardDetectStatus(SD_HOST_BASEADDR, &s_sdCardDetect, true) == kStatus_Success)
//	    {
//	        PRINTF("\r\nCard inserted.\r\n");
//	        /* power on the card */
//	        SD_PowerOnCard(g_sd.host.base, g_sd.usrParam.pwr);
//	    }
//	    else
//	    {
//	        //PRINTF("\r\nCard detect fail.\r\n");
//	        return kStatus_Fail;
//	    }

	    return kStatus_Success;
}

status_t FE_mountDrive(FE_drive drive)
{

	const TCHAR driverNumberBuffer[3U] = {drive + '0', ':', '/'};
	if (!f_mount(&g_fileSystems[drive], driverNumberBuffer, 0U))
	    {
	    	if(!f_chdrive((char const *)&driverNumberBuffer[0U]))
	    	{
	    		return kStatus_Success;

	    	}
	    	else
	    	{
	    		PRINTF("Change drive failed.\r\n");
	    		return kStatus_Fail;
	    	}
	    }
	    else
	    {
	        PRINTF("Mount volume failed.\r\n");
	        return kStatus_Fail;
	    }
}

status_t FE_SetCurrDrive(FE_drive drive)
{
	const TCHAR driverNumberBuffer[3U] = {drive + '0', ':', '/'};

	if(!f_chdrive((char const *)&driverNumberBuffer[0U]))
	{
		return kStatus_Success;

	}
	else
	{
		PRINTF("Change drive failed.\r\n");
		return kStatus_Fail;
	}
}



status_t FE_DirN(const char* path, uint16_t* n, FILINFO* content)
{
    DIR dir;
	status_t retVal;
	FILINFO fno;
	uint16_t i=0;
	if((*n)<=256 && (*n)!=0)
	{
		FRESULT res = f_opendir(&dir, path);                       /* Open the directory */
		if (res == FR_OK)
		{
			for(i=0; i<*n; i++)
			{
				res = f_readdir(&dir, &fno);                   /* Read a directory item */
				if (res != FR_OK || fno.fname[0] == 0)
					break;  		/* Break on error or end of dir */
				*(content+i)=fno;
			}
			*n=i;
		}
		if(res != FR_OK)
			retVal=kStatus_Fail;
		else retVal=kStatus_Success;
	}
	else
		retVal=kStatus_Fail;

	return retVal;
}



/**
 * @brief Open the nth file in the given dir which matches the pattern
 */

FRESULT FE_OpenFileN(const char * path, FIL* fp,FILINFO *fileInfo, BYTE mode, uint8_t n, const char * pattern)
{
    DIR dir;

    FRESULT res = f_findfirst(&dir, fileInfo, path, pattern);

    if(res == FR_OK)
    {
    	while( (n--)>0 && res==FR_OK && fileInfo->fname[0] )
    		 res = f_findnext(&dir, fileInfo);

    	char filePath[255];
    	sprintf(filePath,"%s/%s",path,fileInfo->fname);
    	res = f_open(fp,filePath,mode);

    }
    else
    {

    }

    f_closedir(&dir);

    return res;
}
bool FE_DriveStatus(FE_drive drive)
{
	if(disk_status(g_fileSystems[drive].pdrv)==STA_OK )
		return true;
	else
		return false;
}

uint8_t FE_CountFilesMatching(const char * path, const char * pattern)
{
	uint8_t n = 0;
	DIR dir;
	FILINFO fileInfo;
	FRESULT res = f_findfirst(&dir, &fileInfo, path, pattern);


	if(res == FR_OK && fileInfo.fname[0])
	{
		n++;
		while(f_findnext(&dir, &fileInfo)==FR_OK && fileInfo.fname[0])
			n++;
	}

	f_closedir(&dir);

	return n;
}
