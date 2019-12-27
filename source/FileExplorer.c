/**
 * @file FileExplorer.c
 * @brief
 *
 *
 */

#include "FileExplorer.h"
#include "usb_host_config.h"
#include "usb_host.h"
#include "fsl_sd.h"

#include "stdbool.h"
#include <string.h>
#include "diskio.h"
#include "fsl_debug_console.h"


static void sdStatusChangeFn(bool isInserted, void *userData);

//------

usb_host_handle g_HostHandle;

extern sd_card_t g_sd;

//------

/* File system objects */
static FATFS g_fileSystems[3];

/* SD card detect configuration */
static sdmmchost_detect_card_t cardDetectConfig = {kSDMMCHOST_DetectCardByGpioCD,
													0,
													sdStatusChangeFn,
													sdStatusChangeFn,
													NULL};

// Status of storage drives
static bool sdIsInserted  = false;
static bool sdStatusChanged = false;
static bool usbIsInserted = false;


status_t FE_Init()
{
#ifdef SD_DISK_ENABLE

	/* Save host information. */
	g_sd.host.base = SD_HOST_BASEADDR;
	g_sd.host.sourceClock_Hz = SD_HOST_CLK_FREQ;
	g_sd.usrParam.cd = &cardDetectConfig;

	status_t status = SD_HostInit(&g_sd);

	g_fileSystems[SDDISK].pdrv=SDDISK;
#endif

#ifdef USB_DISK_ENABLE

	    g_fileSystems[USBDISK].pdrv=USBDISK;
#endif

	    return kStatus_Success;
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
	if (!f_mount(&g_fileSystems[drive], driverNumberBuffer, 1U))
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



status_t FE_unmountDrive(FE_drive drive)
{
	const TCHAR driverNumberBuffer[3U] = {drive + '0', ':', '/'};

	if (f_mount(NULL, driverNumberBuffer, 1U)==FR_OK) // TODO: Probar, aca creo que hay que pasarle un 0
	{
			return kStatus_Success;
	}
	else
	{
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
	if(disk_status(g_fileSystems[drive].pdrv)==0 )
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

uint8_t FE_Sort(FE_FILE_SORT_TYPE sort ,const char * path, const char * pattern, uint8_t * indexArray)
{
	DIR dj;
	FRESULT fr = FR_OK;
	uint8_t filesRead = 0;
	char name[MAX_FILES_PER_DIR][FF_LFN_BUF+1];
	char temp[FF_LFN_BUF+1];

	// Find first file
	FILINFO fInfo;
	indexArray[0] = SORTING_END_CHAR;
	fr = f_findfirst(&dj, &fInfo, path, pattern);
	if(!(fr == FR_OK  && fInfo.fname[0]))		// In case path couldnt be opened or no file found
		return filesRead;

	// Copy all file names to array for comparison
	while(fr == FR_OK && fInfo.fname[0])	// While valid file is found
	{
		strcpy(name[filesRead], fInfo.fname);
		filesRead++;
		fr = f_findnext(&dj, &fInfo);
	}
//	indexArray[filesRead] = SORTING_END_CHAR;	// Terminate array with end character

	for(int p = 0 ; p < filesRead ; p++)
		indexArray[p]=p+1;	// Enumerate file names with respective indexes

	// Sort both arrays in parallel
	if(sort == ABC)
	{
		for (int i = 0; i < filesRead - 1 ; i++)
        {
            for (int j = i + 1; j < filesRead; j++)
            {
                if (strcmp(name[i], name[j]) > 0)
                {
                    strcpy(temp, name[i]);
                    strcpy(name[i], name[j]);
                    strcpy(name[j], temp);
                    indexArray[i] = indexArray[j];
                    indexArray[j] = indexArray[i];
                }
            }
        }
	}
	return filesRead;
}


void FE_Tick(void)
{
	/* USB Task */
    USB_HostKhciTaskFunction(g_HostHandle);

	bool currStatus;

//	// Check changes in USB drive
//	currStatus = FE_DriveStatus(FE_USB);
//	if(currStatus != usbStatus)
//	{
//		usbStatus = currStatus;
//		if(usbStatus == true)
//		{
//			PRINTF("USB inserted\n");
//			if(FE_mountDrive(FE_USB)== kStatus_Success)
//			{
//
//				uint8_t k = FE_CountFilesMatching("/","*.mp3");
//
//				PRINTF("There are %d mp3 files in root folder of the USB\n",k);
//
//				MP3_Play("/",0);
//
//			}
//			else
//				PRINTF("Error mounting USB\n");
//		}
//		else
//		{
//			PRINTF("USB removed\n");
//
//		}
//	}

	// Check changes in SD drive
	currStatus = FE_DriveStatus(FE_SD);

	if(sdStatusChanged)
	{
		// ACA NOTIFICAR A LA PARTE GRAFICA!!!
		// GUI_UpdateDriveStatus(FE_SD,sdIsInserted);

		if(sdIsInserted == true)
		{
			PRINTF("SD inserted\n");
			if(FE_mountDrive(FE_SD) == kStatus_Success)
			{
				uint8_t k = FE_CountFilesMatching("/","*.mp3");

				PRINTF("There are %d mp3 files in root folder of the SD\n",k);

				MP3_Play("/",0);

			}
			else
				PRINTF("Error mounting SD\n");
		}

		if(sdIsInserted == false)
		{
			PRINTF("SD removed\n");
			FE_unmountDrive(FE_SD);
		}

		sdStatusChanged = false;
	}
}






static void sdStatusChangeFn(bool isInserted, void *userData)
{
	sdStatusChanged = true;
	sdIsInserted = isInserted;
}
