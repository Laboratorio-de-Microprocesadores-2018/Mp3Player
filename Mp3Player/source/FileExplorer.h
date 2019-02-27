/*
 * FileExplorer.h
 *
 *  Created on: Feb 25, 2019
 *      Author: Diego Juarez
 */

#ifndef FILEEXPLORER_H_
#define FILEEXPLORER_H_

#include "fsl_common.h"
#include "ff.h"
#include "stdbool.h"



status_t FE_Init();
bool FE_FE_check4Drive();
//FILINFO* FE_Dir();
void FE_DeInit();
//FE_OpenFile();
//FE_ReadFile();
//FE_CloseFile();





#endif /* FILEEXPLORER_H_ */
