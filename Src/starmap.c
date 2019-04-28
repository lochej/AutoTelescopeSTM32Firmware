/*
 * starmap_lib.c
 *
 *  Created on: Jan 5, 2019
 *      Author: JLH
 */

#include "starmap.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

StarMap_UTC_t last_utc;
StarMap_Location_t cur_location;

double TSL;

time_t utc_j2000;


void StarMap_init()
{
	struct tm J2000;
	J2000.tm_hour=12;
	J2000.tm_min=0;
	J2000.tm_sec=0;
	J2000.tm_year=2000-1900;
	J2000.tm_mon=0;
	J2000.tm_mday=1;

	utc_j2000=mktime(&J2000);
}


void StarMap_UpdateTSL(StarMap_UTC_t * utc)
{

	time_t utc_current=utc->timestamp;

	//Greenwich TSL à J2000 + nb d'heures angulaires ecoulés depuis j2000
	TSL=18.697374558 + (24.06570982441908/86400.0)*(double)(utc_current - utc_j2000) + cur_location.lon/15.0;

	//TSL=fmod(TSL,24.0);
	TSL -= (double) (((int64_t)TSL)/24)*24;
	//TSL*=15.0;
}



double StarMap_hGetTSL()
{
	return TSL;
}

double StarMap_dGetTSL()
{
	return TSL*15.0;
}

void StarMap_UpdateLocation(StarMap_Location_t location)
{
	cur_location = location;
}

StarMap_EqCoords_t StarMap_getEqCoords(StarMap_SkyCoords_t star)
{
	double ah=StarMap_dGetTSL() - star.ra;

	while(ah<0)
		ah+=360;

	StarMap_EqCoords_t eq={ah,star.dec};
	return eq;
}

//Returns the angle in degrees for an angle represented in xxhyymzz.zzs
StarMap_Error_t StarMap_parseHMS(char * p,StarMap_HMS_t * hms)
{
	StarMap_HMS_t ra =
	{ .h = 0, .m = 0, .s = 0 };

	ra.h = atoi(p);

	if ((p = strchr(p, 'h')) == NULL)
		return STARMAP_ERR;

	p++;
	ra.m = atoi(p);

	if ((p = strchr(p, 'm')) == NULL)
		return STARMAP_ERR;

	p++;
	ra.s = atof(p);

	*hms=ra;

	return STARMAP_OK;

}

StarMap_Error_t StarMap_parseDMS(char * p,StarMap_DMS_t * dms)
{
	StarMap_DMS_t dec =
	{ .d = 0, .m = 0, .s = 0 };

	dec.d = atoi(p);

	if ((p = strchr(p, 'd')) == NULL)
		return STARMAP_ERR;

	p++;

	dec.m = atoi(p);

	if ((p = strchr(p, 'm')) == NULL)
		return STARMAP_ERR;

	p++;

	dec.s = atof(p);

	*dms=dec;

	return STARMAP_OK;
}

double StarMap_hmsToDeg(StarMap_HMS_t hms)
{
	return ((double)hms.h)*15.0 + ((double)hms.m)/4.0 + ((double)hms.s)/240.0;
}

StarMap_HMS_t StarMap_degToHms(double deg)
{
	StarMap_HMS_t hms;

	while(deg < 0)
		deg+=360;

	hms.h=deg/15;

	hms.m=fmod(deg*4,60);

	hms.s=fmod(deg*240,60);

	return hms;
}

StarMap_DMS_t StarMap_degToDms(double deg)
{
	StarMap_DMS_t dms;



	deg = fmod(deg,360);

	dms.d=(int16_t)deg;


	if(deg < 0)
	{
		//Angle is negative
		deg=-deg;
	}

	dms.m=fmod(deg*60,60);

	dms.s=fmod(deg*3600,60);

	return dms;
}

double StarMap_dmsToDeg(StarMap_DMS_t dms)
{

	if(dms.d<0) //Angle is negative
	{
		return ((double)dms.d) - ((double)dms.m)/60.0 - ((double)dms.s)/3600.0;
	}
	return ((double)dms.d) + ((double)dms.m)/60.0 + ((double)dms.s)/3600.0;
}




char * StarMap_ascAngle(double deg,StarMap_AngleDisplay_t as)
{
	static char ascstr[32];
	switch(as)
	{
	case DEGREES:

		snprintf(ascstr,32,"%lf",deg);

		return ascstr;
		break;
	case HMS:

		return StarMap_ascHMS(StarMap_degToHms(deg));
		break;
	case DMS:

		return StarMap_ascDMS(StarMap_degToDms(deg));
		break;
	}

	return ascstr;
}

char * StarMap_ascHMS(StarMap_HMS_t hms)
{
	static char ascstr[32];
	snprintf(ascstr,32,"%dh%02dm%0.3lfs",hms.h,hms.m,hms.s);
	return ascstr;
}

char * StarMap_ascDMS(StarMap_DMS_t dms)
{
	static char ascstr[32];
	snprintf(ascstr,32,"%dd%02dm%0.3lfs",dms.d,dms.m,dms.s);
	return ascstr;
}


