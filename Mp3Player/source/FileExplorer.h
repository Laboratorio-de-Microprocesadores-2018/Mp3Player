

#ifndef FILEEXPLORER_H_
#define FILEEXPLORER_H_

#include "fsl_common.h"
#include "ff.h"
#include "stdbool.h"


#define FE_EOF(fp) 		 f_eof(fp)
#define FE_Error(fp) 	 f_error(fp)
#define FE_Size(fp) 	 f_size(fp)
#define FE_Rewind(fp) 	 f_rewind(fp)
#define FE_RewindDir(dp) f_rewinddir(dp)
#define FE_RmDir(path) 	 f_rmdir(path)
#define FE_Unmount(path) f_unmount(path)

status_t FE_Init();
status_t FE_check4Drive();
status_t FE_mountDrive();
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






#endif /* FILEEXPLORER_H_ */
