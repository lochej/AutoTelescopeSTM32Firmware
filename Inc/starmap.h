/*
 * starmap.h
 *
 *  Created on: Jan 5, 2019
 *      Author: JLH
 */

#ifndef STARMAP_H_
#define STARMAP_H_

#include <time.h>

/**
 * Structure des coordonnées d'une étoile
 * RA = ascension droite de l'étoile a suivre compté positivement vers l'EST le long de l'équateur celeste
 * DEC = Declinaison de l'etoile a suivre compté positivement de 0 à 90 vers le NORD et 0 à -90 vers le SUD
 */
struct StarSkyCoords
{
	double ra;
	double dec;
};

/**
 * Coordonnées équatoriales locales de l'étoile
 */
struct StarEqCoords
{
	double ah;
	double dec;
};

struct StarAzCoords
{
	double azi;
	double alt;
};

struct StarMapLocation
{
	double lon;
	double lat;
};

struct StarMap_UTC
{
	time_t timestamp;
	struct tm time;
};

struct StarMap_HMS
{
	int8_t h;
	int8_t m;
	double s;
};

struct StarMap_DMS
{
	int16_t d;
	int8_t m;
	double s;
};

typedef struct StarEqCoords StarMap_EqCoords_t;

typedef struct StarSkyCoords StarMap_SkyCoords_t;

typedef struct StarMap_UTC StarMap_UTC_t;

typedef struct StarMapLocation StarMap_Location_t;

typedef struct StarMap_HMS StarMap_HMS_t;

typedef struct StarMap_DMS StarMap_DMS_t;


enum StarMap_AngleDisplay_e
{
	DEGREES,
	HMS,
	DMS
};

enum StarMap_Error_e
{
	STARMAP_OK=0,
	STARMAP_ERR
};


typedef enum StarMap_AngleDisplay_e StarMap_AngleDisplay_t;

typedef enum StarMap_Error_e StarMap_Error_t;

void StarMap_init();

void StarMap_UpdateTSL(StarMap_UTC_t * utc);

double StarMap_hGetTSL();

double StarMap_dGetTSL();

void StarMap_UpdateLocation(StarMap_Location_t location);

StarMap_EqCoords_t StarMap_getEqCoords(StarMap_SkyCoords_t star);

StarMap_Error_t StarMap_parseHMS(char * p,StarMap_HMS_t * hms);

StarMap_Error_t StarMap_parseDMS(char * p,StarMap_DMS_t * dms);

double StarMap_hmsToDeg(StarMap_HMS_t hms);

StarMap_HMS_t StarMap_degToHms(double deg);

double StarMap_dmsToDeg(StarMap_DMS_t dms);

StarMap_DMS_t StarMap_degToDms(double deg);

char * StarMap_ascAngle(double deg,StarMap_AngleDisplay_t as);

char * StarMap_ascHMS(StarMap_HMS_t hms);

char * StarMap_ascDMS(StarMap_DMS_t dms);

#endif /* STARMAP_H_ */
