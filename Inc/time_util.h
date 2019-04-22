/*
 * time_util.h
 *
 *  Created on: Jan 21, 2019
 *      Author: JLH
 */

#ifndef TIME_UTIL_H_
#define TIME_UTIL_H_


#include <time.h>
#include "rtc.h"

extern RTC_TimeTypeDef RTCUtil_timeRtc;
extern RTC_DateTypeDef RTCUtil_dateRtc;

void RTCUtil_init();

time_t RTCUtil_time();

void RTCUtil_settime(struct tm datetime);

void RTCUtil_settimestamp(time_t timestamp);

void RTCUtil_saveData(uint32_t backupReg,uint8_t * pData,uint8_t len);

void RCTUtil_readData(uint32_t backupReg,uint8_t * pData, uint8_t len);


#endif /* TIME_UTIL_H_ */
