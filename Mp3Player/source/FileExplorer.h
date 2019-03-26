

#ifndef FILEEXPLORER_H_
#define FILEEXPLORER_H_

#include "fsl_common.h"
#include "ff.h"
#include "stdbool.h"

#define MAX_FILES_PER_DIR 255

#define FE_EOF(fp) 		 f_eof(fp)
#define FE_Error(fp) 	 f_error(fp)
#define FE_Size(fp) 	 f_size(fp)
#define FE_Rewind(fp) 	 f_rewind(fp)
#define FE_RewindDir(dp) f_rewinddir(dp)
#define FE_RmDir(path) 	 f_rmdir(path)
#define FE_Unmount(path) f_unmount(path)

typedef enum{FE_USB=1,FE_SD=2}FE_drive;

status_t FE_Init();
status_t FE_check4Drive();
status_t FE_mountDrive(FE_drive drive);
status_t FE_SetCurrDrive(FE_drive drive);
status_t FE_DirN(const char* path, uint16_t* n, FILINFO* content);
void FE_DeInit();


static inline FRESULT FE_OpenFile( FIL* fp, const TCHAR* path, BYTE mode )
{
	return f_open(fp,path,mode);
}
static inline FRESULT FE_ReadFile (FIL* fp,  void* buff,  UINT btr,  UINT* br)
{
	return f_read(fp,buff,btr,br);
}
static inline FRESULT FE_CloseFile (FIL* fp )
{
	return f_close(fp);
}

static inline FRESULT FE_OpenDir (DIR* dp, const char* path)
{
	return f_opendir(dp,path);
}

bool FE_DriveStatus(FE_drive drive);

/**
 * @brief Open the nth file in the dir which matches the pattern
 */

FRESULT FE_OpenFileN(const char * path, FIL* fp,FILINFO *fileInfo, BYTE mode, uint8_t n, const char * pattern);

/**
 *
 */
uint8_t FE_CountFilesMatching(const char * path, const char * pattern);
#endif /* FILEEXPLORER_H_ */
