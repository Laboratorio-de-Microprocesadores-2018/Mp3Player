#ifndef MP3PLAYER_H_
#define MP3PLAYER_H_
#include "fsl_common.h"


status_t MP3_Init();

void MP3_Play();
void MP3_Tick();

status_t MP3_ComputeSongDuration(char* path, uint32_t * seconds);

#endif /* MP3PLAYER_H_ */
