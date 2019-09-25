[1mdiff --git a/Mp3Player/source/FileExplorer.c b/Mp3Player/source/FileExplorer.c[m
[1mindex d7a204a..659c13b 100644[m
[1m--- a/Mp3Player/source/FileExplorer.c[m
[1m+++ b/Mp3Player/source/FileExplorer.c[m
[36m@@ -22,45 +22,32 @@[m [mextern sd_card_t g_sd;[m
 [m
 //------[m
 [m
[31m-//static FATFS g_fileSystem; /* File system object */[m
[31m-static FATFS g_fileSystems[3]; /*File system objects for RAM, SD, USB,...*/[m
[32m+[m[32m/* File system objects */[m[41m[m
[32m+[m[32mstatic FATFS g_fileSystems[3];[m[41m[m
 [m
[32m+[m[32m/* SD card detect configuration */[m[41m[m
[32m+[m[32mstatic sdmmchost_detect_card_t cardDetectConfig = {kSDMMCHOST_DetectCardByGpioCD,0,NULL,NULL,NULL}; // ACA VAN CALLBACKS PARA CUANDO SE METE Y SACA LA TARJETA[m[41m[m
 [m
[31m-/*! @brief 0 - execute normal fatfs test code; 1 - execute throughput test code */[m
[31m-#define MSD_FATFS_THROUGHPUT_TEST_ENABLE (0U)[m
[31m-[m
[31m-///*! @brief SDMMC host detect card configuration */[m
[31m-//static const sdmmchost_detect_card_t s_sdCardDetect = {[m
[31m-//#ifndef BOARD_SD_DETECT_TYPE[m
[31m-//    .cdType = kSDMMCHOST_DetectCardByGpioCD,[m
[31m-//#else[m
[31m-//    .cdType = BOARD_SD_DETECT_TYPE,[m
[31m-//#endif[m
[31m-//    .cdTimeOut_ms = (0U),//Checks only one time[m
[31m-//};[m
[31m-[m
[31m-[m
[31m-static sdmmchost_detect_card_t cardDetectConfig = {kSDMMCHOST_DetectCardByGpioCD,0,NULL,NULL,NULL};[m
[32m+[m[32m// Status of storage drives[m[41m[m
[32m+[m[32mstatic bool sdStatus = false;[m[41m[m
[32m+[m[32mstatic bool usbStatus = false;[m[41m[m
 [m
 [m
 status_t FE_Init()[m
 {[m
[31m-[m
 #ifdef SD_DISK_ENABLE[m
[31m-	 	g_sd.usrParam.cd = &cardDetectConfig;[m
 [m
[31m-		//if(disk_setUp(SDDISK)!=kStatus_Success)[m
[31m-		//{[m
[31m-		//	return kStatus_Fail;[m
[31m-		//}[m
[32m+[m	[32m/* Save host information. */[m[41m[m
[32m+[m	[32mg_sd.host.base = SD_HOST_BASEADDR;[m[41m[m
[32m+[m	[32mg_sd.host.sourceClock_Hz = SD_HOST_CLK_FREQ;[m[41m[m
[32m+[m	[32mg_sd.usrParam.cd = &cardDetectConfig;[m[41m[m
[32m+[m[41m[m
 [m
[31m-	    g_fileSystems[SDDISK].pdrv=SDDISK;[m
[32m+[m	[32mg_fileSystems[SDDISK].pdrv=SDDISK;[m[41m[m
 #endif[m
[32m+[m[41m[m
 #ifdef USB_DISK_ENABLE[m
[31m-//	    if(disk_setUp(USBDISK)!=kStatus_Success)[m
[31m-//			{[m
[31m-//				return kStatus_Fail;[m
[31m-//			}[m
[32m+[m[41m[m
 	    g_fileSystems[USBDISK].pdrv=USBDISK;[m
 #endif[m
 [m
[36m@@ -110,23 +97,23 @@[m [mstatus_t FE_mountDrive(FE_drive drive)[m
 [m
 	const TCHAR driverNumberBuffer[3U] = {drive + '0', ':', '/'};[m
 	if (!f_mount(&g_fileSystems[drive], driverNumberBuffer, 1U))[m
[31m-	    {[m
[31m-	    	if(!f_chdrive((char const *)&driverNumberBuffer[0U]))[m
[31m-	    	{[m
[31m-	    		return kStatus_Success;[m
[31m-[m
[31m-	    	}[m
[31m-	    	else[m
[31m-	    	{[m
[31m-	    		PRINTF("Change drive failed.\r\n");[m
[31m-	    		return kStatus_Fail;[m
[31m-	    	}[m
[31m-	    }[m
[31m-	    else[m
[31m-	    {[m
[31m-	        PRINTF("Mount volume failed.\r\n");[m
[31m-	        return kStatus_Fail;[m
[31m-	    }[m
[32m+[m	[32m{[m[41m[m
[32m+[m		[32mif(!f_chdrive((char const *)&driverNumberBuffer[0U]))[m[41m[m
[32m+[m		[32m{[m[41m[m
[32m+[m			[32mreturn kStatus_Success;[m[41m[m
[32m+[m[41m[m
[32m+[m		[32m}[m[41m[m
[32m+[m		[32melse[m[41m[m
[32m+[m		[32m{[m[41m[m
[32m+[m			[32mPRINTF("Change drive failed.\r\n");[m[41m[m
[32m+[m			[32mreturn kStatus_Fail;[m[41m[m
[32m+[m		[32m}[m[41m[m
[32m+[m	[32m}[m[41m[m
[32m+[m	[32melse[m[41m[m
[32m+[m	[32m{[m[41m[m
[32m+[m		[32mPRINTF("Mount volume failed.\r\n");[m[41m[m
[32m+[m		[32mreturn kStatus_Fail;[m[41m[m
[32m+[m	[32m}[m[41m[m
 }[m
 [m
 status_t FE_SetCurrDrive(FE_drive drive)[m
