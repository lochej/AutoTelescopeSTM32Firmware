/*
 * telescope_app.h
 *
 *  Created on: Nov 20, 2018
 *      Author: JLH
 */

#ifndef TELESCOPE_APP_H_
#define TELESCOPE_APP_H_


#ifdef __cplusplus
extern "C"
	{
#endif

#include "includes.h"


#define sign(x) ((x)==0 ? 0 : ((x)<0 ? -1 : 1))


extern struct NMEA_Receiver_struct nmea_rcvr;
extern gprmc_t nmea_location_rmc;
extern gpgga_t nmea_location_gga;

extern StarMap_UTC_t startime_utc;
extern StarMap_Location_t geographical_location;
extern StarMap_SkyCoords_t starSkyCoords;
extern StarMap_SkyCoords_t starCorrectionCoords;

extern uint16_t temperature_sensor;

extern double last_h, last_d;

extern uint8_t Flag_enable_NMEA_logging;
extern uint8_t Flag_enable_RTC_GPS_sync ;
extern uint8_t Flag_enable_LOC_GPS_sync ;
extern uint8_t Flag_needs_axis_flipping ;
extern uint8_t Flag_authorize_axis_flipping ;
extern uint8_t Flag_bt_commands_passthrough;
extern uint8_t Flag_enable_periodic_logging;

extern uint8_t b1_pressed;

extern uint32_t inputRead_ms;
extern uint16_t blinkOnTime;
extern uint16_t blinkOffTime;
extern uint32_t blinkref_time ;
extern uint8_t blinklevel;


extern uint8_t Flag_enable_star_follow;



void Telescope_init();
void Telescope_run();


#ifdef __cplusplus
	}
#endif


#endif /* TELESCOPE_APP_H_ */
