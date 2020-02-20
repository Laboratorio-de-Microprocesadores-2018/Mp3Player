/**
 * @file FileExplorer.h
 * @brief
 *
 *
 */

#ifndef FILEEXPLORER_H_
#define FILEEXPLORER_H_

///////////////////////////////////////////////////////////////////////////////
//                             Included header files                         //
///////////////////////////////////////////////////////////////////////////////
#include <stdbool.h>
#include <stdint.h>

#if defined(_WIN64) || defined(_WIN32)
/* Directory API for Windows. */
#include <dirent.h>
//#include "dirent-1.23.2/include/dirent.h"
#include <stdio.h>
#else
#include "fsl_common.h"
#include "ff.h"
#endif

///////////////////////////////////////////////////////////////////////////////
//                       Constants and macro definitions                     //
///////////////////////////////////////////////////////////////////////////////

/* Maximum files per directory supported in one directory. */
#define MAX_FILES_PER_DIR 50

#if defined(_WIN64) || defined(_WIN32)

/* Replace fsl_common.h */
typedef int32_t status_t ;
#define kStatus_Success 0

/* File structure. */
#define FIL		FILE

/* File information structure. */
#define FILINFO dirent

/* API return value type.*/
#define FRESULT int32_t

/* Get file/folder name from a FILINFO. */
#define FE_ENTRY_NAME(filinfo) (filinfo)->d_name

/* Get entry type from a FILINFO. */
#define FE_ENTRY_TYPE(filinfo) (filinfo)->d_type

/* Check if an entry is a folder. */
#define FE_IS_FOLDER(filinfo) (FE_ENTRY_TYPE(filinfo)==DT_DIR)

/* Check if an entry is a file. */
#define FE_IS_FILE(filinfo) (FE_ENTRY_TYPE(filinfo)==DT_REG)

/* */
#define FE_EOF(fp) 		 

/* */
#define FE_Error(fp) 	 

/* */
#define FE_Size(fp) 	 

/* */
#define FE_Rewind(fp) 	 

/* */
#define FE_RewindDir(dp) 

/* */
#define FE_RmDir(path) 	

/* */
#define FE_Unmount(path) 

#else

 /* Get file/folder name from a FILINFO. */
#define FE_ENTRY_NAME(filinfo) (filinfo)->fname

/* Get entry type from a FILINFO. */
#define FE_ENTRY_TYPE(filinfo) (filinfo)->fattrib

/* Check if an entry is a folder. */
#define FE_IS_FOLDER(filinfo) ((FE_ENTRY_TYPE(filinfo)&AM_DIR)==1)

/* Check if an entry is hidden. */
#define FE_IS_HIDDEN(filinfo) ((FE_ENTRY_TYPE(filinfo)&AM_HID)==1)

/* Check if an entry is a file. */
#define FE_IS_FILE(filinfo) ((FE_ENTRY_TYPE(filinfo)&AM_ARC)==1)

/* */
#define FE_EOF(fp) 		 f_eof(fp)

/* */
#define FE_Error(fp) 	 f_error(fp)

/* */
#define FE_Size(fp) 	 f_size(fp)

/* */
#define FE_Rewind(fp) 	 f_rewind(fp)

/* */
#define FE_RewindDir(dp) f_rewinddir(dp)

/* */
#define FE_RmDir(path) 	 f_rmdir(path)

/* */
#define FE_Unmount(path) f_unmount(path)

#endif


///////////////////////////////////////////////////////////////////////////////
//                    Enumerations, structures and typedefs                  //
///////////////////////////////////////////////////////////////////////////////

/* Drive ID. */
typedef enum
{
	FE_SD = 0,
	FE_USB = 1,
} FE_drive;

/* Sort criteria for FE_Sort. */
typedef enum {
	SORT_NONE,			// TODO!
	SORT_ALPHABETIC,
	SORT_SIZE			// TODO!
} FE_SortCriteria_t;







///////////////////////////////////////////////////////////////////////////////
//                                  API                                      //
///////////////////////////////////////////////////////////////////////////////

/*
 * @brief Initialize file explorer.
 */
status_t FE_Init(void);

/*
 * @brief Deinitialize file explorer.
 *
 */
void FE_Deinit(void);

/**
 * @brief Task function should be called periodically in the main loop
 */

void FE_Task(void);

/**
 * @brief Mount (register) drive in filesystem.
 * @param[in] drive
 */
status_t FE_MountDrive(FE_drive drive);

/**
* @brief Unmount drive.
* @param[in] drive
*/
status_t FE_UnmountDrive(FE_drive drive);

/**
 * @brief Set current drive 
 * @param[in] drive ID
 *
 * Used for relative path.
 */
status_t FE_SetCurrDrive(FE_drive drive);

/**
 * @brief Get drive status
 * @param[in] drive ID
 * @return true if drive is available, false otherwise
 */
bool FE_DriveStatus(FE_drive drive);


/**
 * @brief
 * @param dp
 * @param path
 */
#if defined(_WIN64) || defined(_WIN32)
FRESULT FE_OpenDir(DIR** dp, const char* path);
#else
FRESULT FE_OpenDir(DIR* dp, const char* path);
#endif
/**
 * @brief
 * @param dp
 * @param fno
 */
#if defined(_WIN64) || defined(_WIN32)
FRESULT FE_ReadDir(DIR* dp, FILINFO** fno);
#else
FRESULT FE_ReadDir(DIR* dp, FILINFO* fno);
#endif

/**
 * @brief
 * @param dp
 */
FRESULT FE_CloseDir(DIR* dp);

/**
 * @brief
 * @param path
 * @param n
 * @param fileInfo
 */
FRESULT FE_GetFileN(const char* path, uint8_t n, FILINFO* fileInfo);

/**
 * @brief
 * @param fp
 * @param path
 * @param mode
 */
FRESULT FE_OpenFile(FIL** fp, const TCHAR* path, BYTE mode);

/**
 * @brief
 * @param path
 * @param n
 * @param fileInfo
 * @param fp
 * @param mode
 */
FRESULT FE_OpenFileN(const char* path, uint8_t n, FILINFO* fileInfo, FIL* fp, BYTE mode);


/**
 * @brief
 * @param fp
 * @param buff
 * @param btr
 * @param br
 */
FRESULT FE_ReadFile(FIL* fp, void* buff, UINT btr, UINT* br);

/**
 * @brief
 * @param fp
 */
FRESULT FE_CloseFile(FIL* fp);

/**
 * @brief
 * @param path
 * @param n
 */
status_t FE_DirN(const char* path, uint16_t* n, FILINFO* content);

/**
 * @brief Sorts the files in the given path using the specified criteria.
 * @param[in] criteria Sorting criteria.
 * @param[in] path	Null terminated string specifying directory path.
 * @param[out] indexArray Pointer to a index buffer of MAX_FILES_PER_DIR elements
 *
 * The function output is an array of indexed to be used as parameter for FE_OpenFileN()
 * or FE_GetFileN(). So indexArray[0] is the index of the first file, and so on.
 * Folders are placed before files.
 *
 * TODO: Check function stack usage. It may be quite big because of array of FILINFO structs.
 */
int32_t FE_Sort(FE_SortCriteria_t criteria, const char* path, uint32_t* indexArray);

/**
 * @brief Count files matching using specified pattern.
 * @param[in] path Null termiated string specifying directory path.
 * @param[in] pattern Null termiated string with searching pattern
 */
uint8_t FE_CountFiles(const char* path, const char* pattern);


#endif /* FILEEXPLORER_H_ */
