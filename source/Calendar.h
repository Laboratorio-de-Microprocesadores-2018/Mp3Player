#include <time.h>
#include <stdint.h>

#if defined(_WIN64) || defined(_WIN32)
typedef uint32_t status_t;
static time_t startTime;
#else
#include "fsl_common.h"
#endif

//uint8_t monthDurations[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

typedef struct tm CalendarDate_t;

status_t Calendar_Init(void);

void Calendar_GetDate(CalendarDate_t * date);

status_t Calendar_SetDate(CalendarDate_t* date);


 
