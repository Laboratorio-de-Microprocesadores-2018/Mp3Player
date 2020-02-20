#include "Calendar.h"
#include "fsl_rtc.h"

status_t Calendar_Init()
{
	rtc_config_t rtcConfig;
	RTC_GetDefaultConfig(&rtcConfig);
	RTC_Init(RTC, &rtcConfig);
	RTC_SetClockSource(RTC);
	RTC_StartTimer(RTC);

	return kStatus_Success;
}

TM_date Calendar_GetDate()
{
	rtc_datetime_t rtcDate;
	RTC_GetDatetime(RTC, &rtcDate);
	TM_date date={ rtcDate.year, rtcDate.month, rtcDate.day, rtcDate.hour, rtcDate.minute, rtcDate.second};
	return date;
}
status_t Calendar_SetDate(TM_date date)
{
	rtc_datetime_t rtcDate={ date.year, date.month, date.day, date.hour, date.minute, date.second};
	status_t retVal=kStatus_Fail;
	RTC_StopTimer(RTC);
	if (kStatus_Success == RTC_SetDatetime(RTC, &rtcDate))
	{
		retVal=kStatus_Success;
	}
	RTC_StartTimer(RTC);
	return retVal;
}
