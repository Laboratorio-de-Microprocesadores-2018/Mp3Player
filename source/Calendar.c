#include "Calendar.h"


#if defined(_WIN64) || defined(_WIN32)
#include <windows.h>

static time_t startTime;
#else
#include "fsl_rtc.h"
#endif

status_t Calendar_Init()
{
#if defined(_WIN64) || defined(_WIN32)
	time(&startTime);
	return 0;
#else
	rtc_config_t rtcConfig;
	RTC_GetDefaultConfig(&rtcConfig);
	RTC_Init(RTC, &rtcConfig);
	RTC_SetClockSource(RTC);
	// TODO Esperar que el oscilador se estabilice!!
	RTC_StartTimer(RTC);
	return kStatus_Success;
#endif
	
}

void Calendar_GetDate(CalendarDate_t * date)
{
#if defined(_WIN64) || defined(_WIN32)
	SYSTEMTIME time;
	GetLocalTime(&time);
	date->tm_year = time.wYear-2000;
	date->tm_mon = time.wMonth;
	date->tm_mday = time.wDay;
	date->tm_hour = time.wHour;
	date->tm_min = time.wMinute;
	date->tm_sec = time.wSecond;
#else
	rtc_datetime_t rtcDate;
	RTC_GetDatetime(RTC, &rtcDate);

	memset(date,0,sizeof(CalendarDate_t));
	date->tm_year =  rtcDate.year;
	date->tm_mon =  rtcDate.month;
	date->tm_mday =  rtcDate.day;
	date->tm_hour =  rtcDate.hour;
	date->tm_min =  rtcDate.minute;
	date->tm_sec =  rtcDate.second;
#endif

}
status_t Calendar_SetDate(CalendarDate_t* date)
{
#if defined(_WIN64) || defined(_WIN32)
#else
	rtc_datetime_t rtcDate={ date->tm_year, date->tm_mon, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec};

	RTC_StopTimer(RTC);

	status_t retVal = RTC_SetDatetime(RTC, &rtcDate);

	RTC_StartTimer(RTC);

	return retVal;
#endif
}
