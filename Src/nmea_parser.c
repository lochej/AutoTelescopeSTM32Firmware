#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>

#include "nmea_parser.h"



void NMEA_parse_gpgga(char *nmea, gpgga_t *loc)
{
	uint16_t i=0;

	//Not inlined version
#if 1
	for(uint8_t f=0;f<9;f++) //only read the 9 first fields
		//for(uint8_t f=0;f<14;f++)
	{
		while(nmea[i++] != ','); //aller au champ suivant

		switch(f){

		case 0: // Time UTC
		{
			int32_t utc= atoi(nmea+i);

			loc->utc.tm_hour= (utc/10000);
			loc->utc.tm_min= (utc%10000)/100;
			loc->utc.tm_sec= (utc%100);
		}
		break;
		case 1: // Latitude
			loc->latitude=atof(nmea+i);
			break;
		case 2: // N : S
			loc->lat = nmea[i]=='N' || nmea[i]=='S' ? nmea[i] : '\0';
			break;
		case 3: // Longitude
			loc->longitude=atof(nmea+i);
			break;
		case 4: // E : W
			loc->lon = nmea[i]=='E' || nmea[i]=='W' ? nmea[i] : '\0';
			break;
		case 5: // Quality 0 1 2
			loc->quality = atoi(nmea+i);
			break;
		case 6: // Satellites
			loc->satellites = atoi(nmea+i);
			break;
		case 7: // Horizontal dilution of position
			break;

		case 8: //altitude above sea level
			loc->altitude = atof(nmea+i);
			break;
		case 9: // altitude units
			break;
		case 10: // height of geoid
			break;
		case 11: // height unit
			break;
		case 12: // Time in s since last differential data
			break;
		case 13: // diff ref station ID
			break;
		default:
			break;

		}


	}
#endif
#if 0
	//INLINED version
	// Time UTC
	while(nmea[i++] != ',');
	int32_t utc= atoi(nmea+i);
	loc->utc.tm_hour= (utc/10000);
	loc->utc.tm_min= (utc%10000)/100;
	loc->utc.tm_sec= (utc%100);

	// Latitude
	while(nmea[i++] != ',');
	loc->latitude=atof(nmea+i);

	// N : S
	while(nmea[i++] != ',');
	loc->lat = nmea[i]=='N' || nmea[i]=='S' ? nmea[i] : '\0';

	// Longitude
	while(nmea[i++] != ',');
	loc->longitude=atof(nmea+i);

	// E : W
	while(nmea[i++] != ',');
	loc->lon = nmea[i]=='E' || nmea[i]=='W' ? nmea[i] : '\0';

	// Quality 0 1 2
	while(nmea[i++] != ',');
	loc->quality = atoi(nmea+i);

	// Nb satellites
	while(nmea[i++] != ',');
	loc->satellites = atoi(nmea+i);

	// Altitude in Meters
	while(nmea[i++] != ',');
	loc->altitude = atof(nmea+i);

#endif
}

void NMEA_parse_gprmc(char *nmea, gprmc_t *loc)
{

	//Nombre de champs = 11*
	//$GPRMC,180244.000,V,,,,,1.32,105.34,071218,,,E,V*39
	uint16_t i=0;
	for(uint8_t f=0;f<11;f++)
	{
		while(nmea[i++] != ','); //aller au champ suivant

		switch(f){

		case 0: // Time UTC
		{
			int32_t utc= atoi(nmea+i);

			loc->utc.tm_hour= (utc/10000);
			loc->utc.tm_min= (utc%10000)/100;
			loc->utc.tm_sec= (utc%100);
		}
		break;
		case 1: // Validity A-OK V-NOK
			loc->valid= nmea[i]=='A' || nmea[i]=='V' ? nmea[i] : '\0';
			break;
		case 2: // Latitude
			loc->latitude=atof(nmea+i);
			break;
		case 3: // N : S
			loc->lat = nmea[i]=='N' || nmea[i]=='S' ? nmea[i] : '\0';
			break;
		case 4: // Longitude
			loc->longitude=atof(nmea+i);
			break;
		case 5: // E : W
			loc->lon = nmea[i]=='E' || nmea[i]=='W' ? nmea[i] : '\0';
			break;
		case 6: // Speed in knots
			loc->speed = atof(nmea+i);
			break;
		case 7: // True course
			loc->course = atof(nmea+i);
			break;
		case 8: // Date stamp
		{
			int32_t date= atoi(nmea+i);

			loc->utc.tm_mday= (date/10000);
			loc->utc.tm_mon= ((date%10000)/100) - 1;
			loc->utc.tm_year= (date%100) + 2000 - 1900;
		}
		break;

		case 9: // Variation
			break;
		case 10: // E : W
		default:
			break;

		}


	}

}

void NMEA_gprmc_lat_degrees(gprmc_t * loc,double * out)
{
	if (loc->lat == 'N')
	{
		*out=NMEA_ANGLE_TO_DEG(loc->latitude);
	}
	else if (loc->lat == 'S')
	{
		*out= -NMEA_ANGLE_TO_DEG(
				loc->latitude);
	}

}

void NMEA_gprmc_lon_degrees(gprmc_t * loc,double *out)
{
	if (loc->lon == 'E')
	{
		*out= NMEA_ANGLE_TO_DEG(
				loc->longitude);
	}
	else if (loc->lon == 'W')
	{
		*out= -NMEA_ANGLE_TO_DEG(
				loc->longitude);
	}

}

/**
 * Get the message type (GPGGA, GPRMC, etc..)
 *
 * This function filters out also wrong packages (invalid checksum)
 *
 * @param message The NMEA message
 * @return The type of message if it is valid
 */
uint8_t NMEA_get_message_type(const char *message)
{
	uint8_t checksum = 0;
	if ((checksum = NMEA_valid_checksum(message)) != _EMPTY) {
		return checksum;
	}

	if (strstr(message, NMEA_GPGGA_STR) != NULL) {
		return NMEA_GPGGA;
	}

	if (strstr(message, NMEA_GPRMC_STR) != NULL || strstr(message,NMEA_GNRMC_STR) !=NULL) {
		return NMEA_GPRMC;
	}

	return NMEA_UNKNOWN;
}

uint8_t NMEA_valid_checksum(const char *message) {
	uint8_t checksum= (uint8_t)strtol(strchr(message, '*')+1, NULL, 16);

	char p;
	uint8_t sum = 0;
	++message;
	while ((p = *message++) != '*') {
		sum ^= p;
	}

	if (sum != checksum) {
		return NMEA_CHECKSUM_ERR;
	}

	return _EMPTY;
}
