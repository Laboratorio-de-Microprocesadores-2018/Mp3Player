/**
 * @file FileExplorer.h
 * @brief
 *
 *
 */

#ifndef FILEEXPLORER_H_
#define FILEEXPLORER_H_


#if defined(_WIN64) || defined(_WIN32)

#include "dirent-1.23.2/include/dirent.h"
#include <stdio.h>

#define RESULT void

#define FIL		FILE
#define FILINFO dirent

#define FRESULT int32_t

#define FE_ENTRY_NAME(filinfo) filinfo->d_name
#define FE_ENTRY_TYPE(filinfo) filinfo->d_type
#define FE_IS_FOLDER(filinfo) (FE_ENTRY_TYPE(filinfo)==DT_DIR)
#define FE_IS_FILE(filinfo) (FE_ENTRY_TYPE(filinfo)==DT_REG)

#else

#define FE_EOF(fp) 		 f_eof(fp)
#define FE_Error(fp) 	 f_error(fp)
#define FE_Size(fp) 	 f_size(fp)
#define FE_Rewind(fp) 	 f_rewind(fp)
#define FE_RewindDir(dp) f_rewinddir(dp)
#define FE_RmDir(path) 	 f_rmdir(path)
#define FE_Unmount(path) f_unmount(path)

#define FE_ENTRY_NAME(filinfo) filinfo->fname
#define FE_ENTRY_TYPE(filinfo) filinfo->fattrib
#define FE_IS_FOLDER(filinfo) (FE_ENTRY_TYPE(filinfo)==AM_DIR)
#define FE_IS_FILE(filinfo) (FE_ENTRY_TYPE(filinfo)==AM_ARC)

#endif


#if defined(_WIN64) || defined(_WIN32)
#include <stdbool.h>
#include <stdint.h>

typedef int32_t status_t;
#define kStatus_Success 0

#else
#include <stdbool.h>
#include "fsl_common.h"
#include "ff.h"
#endif


#define MAX_FILES_PER_DIR 255
#define SORTING_END_CHAR 0


typedef enum
{
	FE_USB,
	FE_SD
} FE_drive;


typedef enum {
	SORTE_NONE,
	SORT_ALPHABETIC,
	SORT_SIZE
} FE_SortCriteria_t;


/*
 *
 */
status_t FE_Init();

/*
 *
 */
void FE_DeInit();

/**
 * @brief Task function should be called periodically in the main loop
 */

void FE_Task(void);

/*
 *
 */
status_t FE_check4Drive();

/*
 *
 */
status_t FE_mountDrive(FE_drive drive);
status_t FE_unmountDrive(FE_drive drive);

/*
 *
 */
status_t FE_SetCurrDrive(FE_drive drive);


#if defined(_WIN64) || defined(_WIN32)
	

	/*
		*
		*/
status_t FE_DirN(const char* path, uint16_t* n, FILINFO* content);



FRESULT FE_OpenFile(FIL** fp, const TCHAR* path, BYTE mode);
FRESULT FE_ReadFile(FIL* fp, void* buff, UINT btr, UINT* br);

FRESULT FE_CloseFile(FIL* fp);

FRESULT FE_OpenDir(DIR** dp, const char* path);
FRESULT FE_ReadDir(DIR* dp, FILINFO** fno );
FRESULT FE_CloseDir(DIR*dp);

/**
	* @brief Open the nth file in the dir which matches the pattern
	*/
FRESULT FE_OpenFileN(const char* path, FIL* fp, FILINFO* fileInfo, BYTE mode, uint8_t n, const char* pattern);

	
#else

/*
 *
 */
status_t FE_DirN(const char* path, uint16_t* n, FILINFO* content);



static inline FRESULT FE_OpenFile(FIL* fp, const TCHAR* path, BYTE mode)
{
	return f_open(fp, path, mode);
}

static inline FRESULT FE_ReadFile(FIL* fp, void* buff, UINT btr, UINT* br)
{
	return f_read(fp, buff, btr, br);
}

static inline FRESULT FE_CloseFile(FIL* fp)
{
	return f_close(fp);
}

static inline FRESULT FE_OpenDir(DIR* dp, const char* path)
{
	return f_opendir(dp, path);
}


/**
 * @brief Open the nth file in the dir which matches the pattern
 */

FRESULT FE_OpenFileN(const char* path, FIL* fp, FILINFO* fileInfo, BYTE mode, uint8_t n, const char* pattern);

#endif

bool FE_DriveStatus(FE_drive drive);


/**
 *
 */
uint8_t FE_CountFilesMatching(const char * path, const char * pattern);


/**
 * @brief Sorts the files with a given extension from path to indexArray following a sort criteria.
 */
uint8_t FE_Sort(FE_SortCriteria_t sort ,const char * path, const char * pattern, uint8_t * indexArray);


#endif /* FILEEXPLORER_H_ */
