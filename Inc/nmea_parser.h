#ifndef _NMEA_H_
#define _NMEA_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>

#define _EMPTY 0x00
#define NMEA_GPRMC 0x01
#define NMEA_GPRMC_STR "$GPRMC"
#define NMEA_GPGGA 0x02
#define NMEA_GPGGA_STR "$GPGGA"
#define NMEA_GNRMC 0x03
#define NMEA_GNRMC_STR "$GNRMC"

#define NMEA_UNKNOWN 0x00
#define _COMPLETED 0x03

#define NMEA_CHECKSUM_ERR 0x80
#define NMEA_MESSAGE_ERR 0xC0

#define NMEA_ANGLE_TO_DEG(_ang) ((int32_t) ((_ang) / 100) + fmod((_ang), 100) / 60)

struct gpgga {
    // Latitude eg: 4124.8963 (XXYY.ZZKK.. DEG, MIN, SEC.SS)
    double latitude;
    // Latitude eg: N
    char lat;
    // Longitude eg: 08151.6838 (XXXYY.ZZKK.. DEG, MIN, SEC.SS)
    double longitude;
    // Longitude eg: W
    char lon;
    // Quality 0, 1, 2
    uint8_t quality;
    // Number of satellites: 1,2,3,4,5...
    uint8_t satellites;
    // Altitude eg: 280.2 (Meters above mean sea level)
    double altitude;
    struct tm utc;

};
typedef struct gpgga gpgga_t;

struct gprmc {
    double latitude;
    char lat;
    double longitude;
    char lon;
    double speed;
    double course;
    char valid;
    struct tm utc;
};
typedef struct gprmc gprmc_t;



uint8_t NMEA_get_message_type(const char *);
uint8_t NMEA_valid_checksum(const char *);
void NMEA_parse_gpgga(char *, gpgga_t *);
void NMEA_parse_gprmc(char *, gprmc_t *);
void NMEA_gprmc_lat_degrees(gprmc_t * loc,double * out);
void NMEA_gprmc_lon_degrees(gprmc_t * loc,double * out);

#endif
