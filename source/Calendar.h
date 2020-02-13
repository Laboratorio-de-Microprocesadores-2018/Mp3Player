#include "fsl_common.h"

typedef struct
{
    uint16_t year;  /*!< Range from 1970 to 2099.*/
    uint8_t month;  /*!< Range from 1 to 12.*/
    uint8_t day;    /*!< Range from 1 to 31 (depending on month).*/
    uint8_t hour;   /*!< Range from 0 to 23.*/
    uint8_t minute; /*!< Range from 0 to 59.*/
    uint8_t second; /*!< Range from 0 to 59.*/
} TM_date;

status_t Calendar_Init(void);
TM_date Calendar_GetDate(void);
status_t Calendar_SetDate(TM_date date);

