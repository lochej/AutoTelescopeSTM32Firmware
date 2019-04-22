/*
 * time_util.c
 *
 *  Created on: Jan 21, 2019
 *      Author: JLH
 */

#if 0
#include "time_util.h"
#include "rtc.h"
#include <string.h>
#endif

#include "includes.h"


RTC_TimeTypeDef RTCUtil_timeRtc;
RTC_DateTypeDef RTCUtil_dateRtc;
struct tm timetm;

void RTCUtil_init()
{
}

time_t RTCUtil_time()
{

	HAL_RTC_GetTime(&hrtc, &RTCUtil_timeRtc, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &RTCUtil_dateRtc, RTC_FORMAT_BIN);

	timetm.tm_hour=RTCUtil_timeRtc.Hours;
	timetm.tm_sec=RTCUtil_timeRtc.Seconds;
	timetm.tm_min=RTCUtil_timeRtc.Minutes;

	timetm.tm_mday=RTCUtil_dateRtc.Date;
	timetm.tm_mon=RTCUtil_dateRtc.Month - 1;
	timetm.tm_year=2000+RTCUtil_dateRtc.Year-1900;

	return mktime(&timetm);
}

void RTCUtil_settime(struct tm datetime)
{
	RTC_DateTypeDef d;
	RTC_TimeTypeDef t;
	d.Date=datetime.tm_mday;

	d.Month=datetime.tm_mon+1;

	d.Year=datetime.tm_year+1900 - 2000;


	int c=(d.Month <= 2 ? 1:0);
	int a=2000+d.Year - c;
	int m=d.Month + 12*c - 2;
	d.WeekDay = (d.Date + a + a/4 - a/100 + a/400 + (31*m)/12)%7 ;

	d.WeekDay=d.WeekDay == 0 ? 7 : d.WeekDay;

	HAL_RTC_SetDate(&hrtc, &d,RTC_FORMAT_BIN);


	t.Hours=datetime.tm_hour;


	t.Minutes=datetime.tm_min;

	t.Seconds=datetime.tm_sec;
	t.TimeFormat=RTC_HOURFORMAT_24;
	t.StoreOperation=RTC_STOREOPERATION_RESET;
	t.DayLightSaving=RTC_DAYLIGHTSAVING_NONE;

	HAL_RTC_SetTime(&hrtc, &t,RTC_FORMAT_BIN);
}

void RTCUtil_settimestamp(time_t timestamp)
{
	struct tm * t;
	t=gmtime(&timestamp);
	RTCUtil_settime(*t);
}
void RTCUtil_saveData(uint32_t backupReg,uint8_t * pData,uint8_t len)
{

	//memcpy(pData,&(hrtc.Instance->BKP0R) + backupReg*4,len);


	for(uint8_t i=0; i<len/4; i++)
	{
		uint32_t tmp=0;
		if(i == (len/4 - 1) && 0)
		{
			memcpy(pData+i*4,&tmp,len - i*4);
		}
		else
		{
			tmp = *(uint32_t *) (pData + 4*i);
		}
		HAL_RTCEx_BKUPWrite(&hrtc, backupReg+i, tmp);


	}

}

void RCTUtil_readData(uint32_t backupReg,uint8_t * pData, uint8_t len)
{


	for(uint8_t i=0; i<len/4; i++)
	{

		if(i == (len/4 - 1) && 0)
		{
			uint32_t tmp;
			tmp = HAL_RTCEx_BKUPRead(&hrtc,backupReg + i);
			memcpy(&tmp,pData+i*4,len - i*4);
		}
		else
		{
			uint32_t tmp=HAL_RTCEx_BKUPRead(&hrtc,backupReg + i);
			*((uint32_t *)(pData + i*4)) = tmp;
		}


	}



}





