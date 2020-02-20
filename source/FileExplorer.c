/**
 * @file FileExplorer.c
 * @brief
 *
 *
 */


///////////////////////////////////////////////////////////////////////////////
//                             Included header files                         //
///////////////////////////////////////////////////////////////////////////////

#include "FileExplorer.h"
#include <string.h>

#if defined(_WIN64) || defined(_WIN32)
#define FR_OK 0
#define PRINTF printf
#else

#include "fsl_debug_console.h"
#include "fsl_sysmpu.h"
#include "fsl_sd.h"
#include "fsl_sd_disk.h"
#include "usb_host_config.h"
#include "usb_host.h"
#include "diskio.h" // TODO: Al final pienso que no deberiamos incluir ni llamar funciones de ac�

#endif



///////////////////////////////////////////////////////////////////////////////
//                   Local function prototypes ('static')                    //
///////////////////////////////////////////////////////////////////////////////

static int SortAlphaCompare(void* a, void* b);
static void sdStatusChangeFn(bool isInserted, void* userData);
static void usbStatusChangeFn(bool isInserted, void* userData);

///////////////////////////////////////////////////////////////////////////////
//                   Local variable definitions ('static')                   //
///////////////////////////////////////////////////////////////////////////////

// Status of storage drives
static bool sdIsInserted = false;
static bool sdStatusChanged = false;

static bool usbIsInserted = false;
static bool usbStatusChanged = false;

#if defined(_WIN64) || defined(_WIN32)

#else

usb_host_handle g_HostHandle;
//extern sd_card_t g_sd;
/* SD card detect configuration */
static sdmmchost_detect_card_t cardDetectConfig = { kSDMMCHOST_DetectCardByGpioCD,
													0,
													sdStatusChangeFn,
													sdStatusChangeFn,
													NULL };
/* File system objects */
static FATFS g_fileSystems[3];

#endif





///////////////////////////////////////////////////////////////////////////////
//                                  API                                      //
///////////////////////////////////////////////////////////////////////////////

status_t FE_Init(void)
{
#if defined(_WIN64) || defined(_WIN32)
	return 0;
#else

	/* Save host information. */
	g_sd.host.base = SD_HOST_BASEADDR;
	g_sd.host.sourceClock_Hz = SD_HOST_CLK_FREQ;
	g_sd.usrParam.cd = &cardDetectConfig;

	status_t status = SD_HostInit(&g_sd);

	g_fileSystems[SDDISK].pdrv=SDDISK;

	g_fileSystems[USBDISK].pdrv=USBDISK;

	SYSMPU_Enable(SYSMPU, false);

	return status;
#endif
}


void FE_Deinit(void)
{
#if defined(_WIN64) || defined(_WIN32)
#else
	SD_HostDeinit(&g_sd);
#endif
}




void FE_Task(void)
{
#if defined(_WIN64) || defined(_WIN32)
#else

	/* USB Task */
	//USB_HostKhciTaskFunction(hostHandle);

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
	//				uint8_t k = FE_CountFiles("/","*.mp3");
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
	if (sdStatusChanged)
	{

		if (sdIsInserted == true)
		{
			PRINTF("SD inserted\n");
			if (FE_MountDrive(FE_SD) == kStatus_Success)
			{
				uint8_t k = FE_CountFiles("/", "*.mp3");

				PRINTF("There are %d mp3 files in root folder of the SD\n", k);

			}
			else
				PRINTF("Error mounting SD\n");
		}

		if (sdIsInserted == false)
		{
			PRINTF("SD removed\n");
			FE_UnmountDrive(FE_SD);
		}

		GUI_UpdateDriveStatus(FE_SD,sdIsInserted);

		sdStatusChanged = false;
	}
#endif
}


status_t FE_MountDrive(FE_drive drive)
{
#if defined(_WIN64) || defined(_WIN32)
	return kStatus_Success;
#else
	const TCHAR driverNumberBuffer[4U] = { '0' + drive , ':', '/', '\0' };

	FRESULT res = f_mount(&g_fileSystems[drive], driverNumberBuffer, 1U);

	if (res == FR_OK)
	{
		if (!f_chdrive((char const*)& driverNumberBuffer[0U]))
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
#endif
}


status_t FE_UnmountDrive(FE_drive drive)
{
#if defined(_WIN64) || defined(_WIN32)
	return kStatus_Success;
#else
	const TCHAR driverNumberBuffer[3U] = { drive + '0', ':', '/' };

	if (f_mount(NULL, driverNumberBuffer, 1U) == FR_OK) // TODO: Probar, aca creo que hay que pasarle un 0
	{
		return kStatus_Success;
	}
	else
	{
		return kStatus_Fail;
	}
#endif
}


status_t FE_SetCurrDrive(FE_drive drive)
{
#if defined(_WIN64) || defined(_WIN32)
#else
	const TCHAR driverNumberBuffer[3U] = { drive + '0', ':', '/' };

	if (!f_chdrive((char const*)& driverNumberBuffer[0U]))
	{
		return kStatus_Success;

	}
	else
	{
		PRINTF("Change drive failed.\r\n");
		return kStatus_Fail;
	}
#endif
}


bool FE_DriveStatus(FE_drive drive)
{
#if defined(_WIN64) || defined(_WIN32)
	if (drive == FE_SD)
		return true;
	else
		return false;
#else
	if(drive == SDDISK)
		return sdIsInserted;
	else if(drive == USBDISK)
		return usbIsInserted;

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

#endif

	return false;
}



#if defined(_WIN64) || defined(_WIN32)
FRESULT FE_OpenDir(DIR** dp, const char* path)
{
	*dp = opendir(path);

	if (*dp != NULL)
		return 0;
	else
		return -1;
}
#else
FRESULT FE_OpenDir(DIR* dp, const char* path)
{
	return f_opendir(dp, path);
}
#endif




#if defined(_WIN64) || defined(_WIN32)
FRESULT FE_ReadDir(DIR** dp, FILINFO** fno)
{
	*fno = readdir(*dp);

	if (*fno != NULL)
		return 0;
	else
		return -1;
}
#else
FRESULT FE_ReadDir(DIR* dp, FILINFO* fno)
{
	FRESULT res = f_readdir(dp,fno);

	if(fno->fname[0] == '\0')
		return FR_NO_FILE;
	else
		return res;
}
#endif



FRESULT FE_CloseDir(DIR* dp)
{
#if defined(_WIN64) || defined(_WIN32)
	return closedir(dp);
#else
	return f_closedir(dp);
#endif
}


FRESULT FE_GetFileN(const char* path, uint8_t n, FILINFO* fileInfo)
{
	if (n > MAX_FILES_PER_DIR)
		return -1;

	/* Try to open directory. */
#if defined(_WIN32) || defined(_WIN64)
	DIR * dr = NULL;
	FILINFO* de = NULL;
#else
	DIR dr;
	FILINFO de;
#endif

	FRESULT res = FE_OpenDir(&dr, path);

	if(res == FR_OK)
	{
		for(int i=0; i<=n; i++)
		{
			res = FE_ReadDir(&dr, &de);

			if(res != FR_OK)
			{
				PRINTF("FE_GetFileN() Error: %d\n", res);
				break;
			}
			//if (strcmp(FE_ENTRY_NAME(de), ".") != 0
			//	&& strcmp(FE_ENTRY_NAME(de), "..") != 0)
		}
	}
	else
	{
		PRINTF("FE_GetFileN() Error: %d\n", res);
	}

#if defined(_WIN32) || defined(_WIN64)
	memcpy(fileInfo, de, sizeof(FILINFO));
	FE_CloseDir(dr);
#else
	memcpy(fileInfo, &de, sizeof(FILINFO));
	FE_CloseDir(&dr);
#endif

	return res;
}


FRESULT FE_OpenFile(FIL** fp, const TCHAR* path, BYTE mode)
{
#if defined(_WIN64) || defined(_WIN32)
	*fp = fopen(path, mode);
	if (*fp != NULL)
		return 0;
#else
	return f_open(*fp, path, mode);
#endif
}


FRESULT FE_OpenFileN(const char* path, uint8_t n, FILINFO* fileInfo, FIL* fp, BYTE mode)
{
#if defined(_WIN64) || defined(_WIN32)


#else
	FRESULT res = FE_GetFileN(path, n, fileInfo);

	if (res == FR_OK)
	{
		char filePath[255];
		sprintf(filePath,"%s/%s",path,FE_ENTRY_NAME(fileInfo));
		return f_open(fp, filePath, mode);
	}
	return res;

#endif
}


FRESULT FE_ReadFile(FIL* fp, void* buff, UINT btr, UINT* br)
{
#if defined(_WIN64) || defined(_WIN32)
	size_t nElems = fread(buff, sizeof(unsigned int), btr / sizeof(unsigned int), fp);

	*br = nElems * sizeof(unsigned int);

	if (*br == btr)
		return 0;
	else
		return -1;
#else
	return f_read(fp, buff, btr, br);
#endif
}


FRESULT FE_CloseFile(FIL* fp)
{
#if defined(_WIN64) || defined(_WIN32)
	return fclose(fp);
#else
	return f_close(fp);
#endif
}


status_t FE_DirN(const char* path, uint16_t* n, FILINFO* content)
{
#if defined(_WIN64) || defined(_WIN32)
#else
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
#endif
}


int32_t FE_Sort(FE_SortCriteria_t sort, const char* path, uint32_t* indexArray)
{

	/* Try to open directory. */
#if defined(_WIN32) || defined(_WIN64)
	DIR * dr;
	FILINFO* de;
#else
	DIR dr;
	FILINFO de;
#endif

	FRESULT res = FE_OpenDir(&dr, path);

	if (res != 0)
	{
		PRINTF("Path not found: [%s]. Error code %d\n", path,res);
		return -1;
	}

	uint8_t entriesCount = 0;

	/* Now iterate through folder storing files info in an array. */

	FILINFO dirEntries[MAX_FILES_PER_DIR];

	while (FE_ReadDir(&dr, &de)==FR_OK && entriesCount < MAX_FILES_PER_DIR)
	{

#if defined(_WIN32) || defined(_WIN64)
		memcpy(&dirEntries[entriesCount], de, sizeof(FILINFO));
#else
		memcpy(&dirEntries[entriesCount], &de, sizeof(FILINFO));
#endif
		entriesCount++;
	}

#if defined(_WIN32) || defined(_WIN64)
	FE_CloseDir(dr);
#else
	FE_CloseDir(&dr);
#endif


	/* Build array of pointers. */
	FILINFO* dirEntriesP[MAX_FILES_PER_DIR];
	for (int i = 0; i < entriesCount; i++)
	{
		dirEntriesP[i] = &dirEntries[i];
#if defined(_WIN32) || defined(_WIN64)
		PRINTF("%d, ", (uint8_t)(dirEntriesP[i] - &dirEntries[0]));
#endif
	}
	PRINTF("\n");


	/* Sort them. */
	qsort(dirEntriesP, entriesCount, sizeof(FILINFO*), SortAlphaCompare);


	for (int i = 0; i < entriesCount; i++)
	{
		indexArray[i] = (dirEntriesP[i] - &dirEntries[0]);
#if defined(_WIN32) || defined(_WIN64)
		PRINTF("%d, ", (uint8_t)(dirEntriesP[i] - &dirEntries[0]));
#endif
	}
	PRINTF("\n\n");

	return entriesCount;
}


uint8_t FE_CountFiles(const char * path, const char * pattern)
{
#if defined(_WIN64) || defined(_WIN32)
#else
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
	else
		PRINTF("FE_CountFiles() Error code %d\n", res);
	f_closedir(&dir);

	return n;
#endif
}

///////////////////////////////////////////////////////////////////////////////
//                   Local function definition ('static')                    //
///////////////////////////////////////////////////////////////////////////////

static int SortAlphaCompare(void* a, void* b)
{
	FILINFO* entryA = *(FILINFO**)(a);
	FILINFO* entryB = *(FILINFO**)(b);
	
	if (FE_IS_FOLDER(entryA) && FE_IS_FILE(entryB))
		return -1;
	else if (FE_IS_FILE(entryA) && FE_IS_FOLDER(entryB))
		return 1;
	else
		return strcmp(FE_ENTRY_NAME(entryA), FE_ENTRY_NAME(entryB));
}


static void sdStatusChangeFn(bool isInserted, void *userData)
{
	sdStatusChanged = true;
	sdIsInserted = isInserted;
}

static void usbStatusChangeFn(bool isInserted, void* userData)
{
	usbStatusChanged = true;
	usbIsInserted = isInserted;
}
