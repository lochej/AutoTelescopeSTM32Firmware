/*
 * telescope_app.cpp
 *
 *  Created on: Nov 20, 2018
 *      Author: JLH
 */

#if 0
#include "telescope_app.h"
#include "stm32l4xx_hal.h"
#include "main.h"
#include "Arduino_HAL.h"
#include "uart_driver.h"
#include "nmea_receiver.h"
#include "nmea_parser.h"
#include "rtc.h"
#include "string.h"
#include "adc.h"

#include "stepper_adapt2.h"
#include "starmap.h"
#include "time_util.h"
#endif

#include "includes.h"

#ifdef USE_ANALOG_LIMIT_SWITCHES
void Process_ReadInputs();
#endif

void Process_Receive_Cmd();
void Process_NMEA();
void Process_Main_LED();
void Process_Buttons();
void Process_Steppers();
void Process_Periodic_Logging();

struct NMEA_Receiver_struct nmea_rcvr;
gprmc_t nmea_location_rmc;
gpgga_t nmea_location_gga;

StarMap_UTC_t startime_utc;
StarMap_Location_t geographical_location;
StarMap_SkyCoords_t starSkyCoords;
StarMap_SkyCoords_t starCorrectionCoords;
StarMap_EqCoords_t starEqCoords;

uint16_t temperature_sensor = 0;

double last_h, last_d;

uint8_t Flag_enable_NMEA_logging = 0;
uint8_t Flag_enable_RTC_GPS_sync = 1;
uint8_t Flag_enable_LOC_GPS_sync = 1;
uint8_t Flag_needs_axis_flipping = 0;
uint8_t Flag_authorize_axis_flipping = 0;

uint8_t Flag_enable_periodic_logging = 1;

uint8_t b1_pressed = 0;

uint32_t inputRead_ms = 0;
uint16_t blinkOnTime = 50;
uint16_t blinkOffTime = 100;
uint32_t blinkref_time = 0;
uint8_t blinklevel = HIGH;

uint8_t Flag_bt_commands_passthrough = 0;

uint8_t Flag_enable_star_follow = 0;

#ifdef USE_ANALOG_LIMIT_SWITCHES

volatile uint8_t adc_values[3]=
{	0};
volatile uint8_t adc_new_data =0;

#endif

void Telescope_init()
{

	RTCUtil_init();
	TimerCycInit();

#ifdef USE_ANALOG_LIMIT_SWITCHES
	HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
#endif

	Stepper_initSteppers();

	NMEA_receiver_init(&nmea_rcvr);
	NMEA_receiver_reset(&nmea_rcvr);

#if UART_DRIVER_USE_DYN_ALLOC
	Serial_STLK = Serial_new(&huart2, MAX_RX_BUFFER_UART2, MAX_TX_BUFFER_UART2);
	Serial_GPS = Serial_new(&huart4, MAX_RX_BUFFER_UART4, MAX_TX_BUFFER_UART4);
	Serial_BT = Serial_new(&huart3, 32, 128);
#else
	Serial_STLK = Serial_init(&huart2, &Fifo_Rx_UART2, &Fifo_Tx_UART2);
	Serial_GPS = Serial_init(&huart4, &Fifo_Rx_UART4, &Fifo_Tx_UART4);
#endif

	StarMap_init();

	memset(&geographical_location, 0, sizeof(geographical_location));
	memset(&startime_utc, 0, sizeof(startime_utc));
	memset(&starSkyCoords, 0, sizeof(starSkyCoords));
	memset(&starCorrectionCoords, 0, sizeof(starCorrectionCoords));

	//Restorer la dernière position enregistrée
	RCTUtil_readData(RTC_BKP_DR0, (uint8_t *) &geographical_location,
			sizeof(geographical_location));
	StarMap_UpdateLocation(geographical_location);

}

void Telescope_run()
{

	while (1)
	{

#ifdef USE_ANALOG_LIMIT_SWITCHES
		Process_ReadInputs();
#endif

#if 0

		while(Serial_Available(&Serial_BT))
		{
			char c=0;
			Serial_Read(&Serial_BT,&c);
			//Serial_writeBuf(&Serial_BT,&c,1);
			Serial_writeBuf(&Serial_STLK,&c,1);

		}

		while(Serial_Available(&Serial_STLK))
		{
			char c=0;
			Serial_Read(&Serial_STLK,&c);
			Serial_writeBuf(&Serial_BT,&c,1);

		}
#endif
#if 1
		Process_Receive_Cmd();

		//Log NMEA frames directly to com port
#if 1
		Process_NMEA();
#else
		while(Serial_Available(&Serial_GPS))
		{
			char c=0;
			Serial_Read(&Serial_GPS,&c);
			Serial_printf(&Serial_STLK,"%c",c);
		}
#endif
		Process_Buttons();

		Process_Main_LED();

		//Process_Steppers();

		Process_Periodic_Logging();

#endif
	}

}

void Process_Periodic_Logging()
{

	static uint32_t reftimertc = 0;
	StarMap_HMS_t hmstsl;
	StarMap_DMS_t dmslat;
	StarMap_DMS_t dmslon;
	StarMap_HMS_t hmsah;
	StarMap_DMS_t dmsdec;

	if (Flag_enable_periodic_logging)
		if (millis() - reftimertc >= 1000)
		{
			reftimertc = millis();
			uint32_t refcyc = TimerCycCount();

			startime_utc.timestamp = RTCUtil_time();

			StarMap_UpdateTSL(&startime_utc);

			hmstsl = StarMap_degToHms(StarMap_dGetTSL());
			dmslat = StarMap_degToDms(geographical_location.lat);
			dmslon = StarMap_degToDms(geographical_location.lon);
			//hmsah = StarMap_degToHms(stepperAh2.pos * stepperAh2.degreesPerSteps);
			//dmsdec = StarMap_degToDms(stepperDec2.pos * stepperDec2.degreesPerSteps);
			hmsah = StarMap_degToHms(Stepper_getRelativePos(&stepperAh2));
			dmsdec = StarMap_degToDms(Stepper_getRelativePos(&stepperDec2));

			//Serial_printf(&Serial_UART2,"RTC %02dh%02dm%02ds %02d/%02d/%02d/%1d \r\n TSL %dh%lf \r\n",timertc.Hours,timertc.Minutes,timertc.Seconds,date.Date,date.Month,date.Year,date.WeekDay,(int)StarMap_GetTSL(),fmod(StarMap_GetTSL()*60,60));

			Serial_printf(&Serial_STLK, "RTC %02d/%02d/%d %02d:%02d:%02d\n"
					"TSL: %dh%dm%0.3lfs | %lf\n"
					"lat: %d°%02d'%0.3lf\" "
					"lon: %d°%02d'%0.3lf\" \n"
					"Dec: %d°%02d'%0.3lf\" "
					"Ah: %dh%dm%0.3lfs \n\n", RTCUtil_dateRtc.Date,
					RTCUtil_dateRtc.Month, RTCUtil_dateRtc.Year,
					RTCUtil_timeRtc.Hours, RTCUtil_timeRtc.Minutes,
					RTCUtil_timeRtc.Seconds, hmstsl.h, hmstsl.m, hmstsl.s,
					StarMap_dGetTSL(), dmslat.d, dmslat.m, dmslat.s, dmslon.d,
					dmslon.m, dmslon.s, dmsdec.d, dmsdec.m, dmsdec.s, hmsah.h,
					hmsah.m, hmsah.s);

			/*
			 Serial_printf(&Serial_UART2, "RTC %s"
			 "TSL %dh%dm%0.3lfs | %lf\n"
			 "lat: %d°%02d'%0.3lf\" "
			 "lon: %d°%02d'%0.3lf\" \n\n",
			 asctime(gmtime(&(startime_utc.timestamp))),
			 hmstsl.h, hmstsl.m, hmstsl.s, StarMap_dGetTSL(), dmslat.d, dmslat.m, dmslat.s,
			 dmslon.d, dmslon.m, dmslon.s);
			 */
			/*
			 Serial_printf(&Serial_UART2,"RTC %s",asctime(gmtime(&(startime_utc.timestamp))));
			 Serial_printf(&Serial_UART2,"TSL %s | %lf\n",StarMap_ascAngle(StarMap_dGetTSL(), HMS),StarMap_dGetTSL());
			 Serial_printf(&Serial_UART2,"lat: %s ",StarMap_ascAngle(geographical_location.lat, DMS));
			 Serial_printf(&Serial_UART2,"lon: %s \n\n",StarMap_ascAngle(geographical_location.lon, DMS));
			 */
			Serial_printf(&Serial_STLK, "Log time: %d\n",
					TimerCycCount() - refcyc);

			Process_Steppers();

		}
}

#ifdef USE_ANALOG_LIMIT_SWITCHES
uint8_t NAMUR_conversion(uint8_t value)
{
	return value/80;
}

void Process_ReadInputs()
{
	static uint32_t ref_time=0;
	static uint32_t ref_cyc=0;

	if(millis() - ref_time >= 200)
	{
		ref_time=millis();

		ref_cyc=*DWT_CYCCNT;
		HAL_ADC_Start_DMA(&hadc1,adc_values,3);

#if 0
		uint16_t adc_read=0;

		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1,0xFF);
		adc_read=HAL_ADC_GetValue(&hadc1)*3300UL/TEMPSENSOR_CAL_VREFANALOG;

		int32_t temp=map(adc_read, *(TEMPSENSOR_CAL1_ADDR), *TEMPSENSOR_CAL2_ADDR,TEMPSENSOR_CAL1_TEMP,TEMPSENSOR_CAL2_TEMP);
		Serial_printf(&Serial_STLK,"Temp:%d  [%d : %d]\t %ld\n\n",adc_read,*(TEMPSENSOR_CAL1_ADDR),*TEMPSENSOR_CAL2_ADDR,temp);
#endif

	}

	if(adc_new_data)
	{
		ref_cyc=*DWT_CYCCNT - ref_cyc;
		for(uint8_t i=0;i<3;i++)
		{
			Serial_printf(&Serial_STLK,"Ch:%d Val=%d NAMUR=%d\n",i,adc_values[i], NAMUR_conversion(adc_values[i]));
		}
		Serial_printf(&Serial_STLK, "TIME:%ld\r\n",ref_cyc);

		adc_new_data=0;
	}

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef * hadc)
{

	if(hadc->Instance==ADC1)
	{
		adc_new_data=1;

	}
}
#endif

void Process_NMEA()
{
	NMEA_receiver_task(&nmea_rcvr);

	if (NMEA_receiver_frame_ready(&nmea_rcvr))
	{

		switch (NMEA_get_message_type((char *) (nmea_rcvr.buffer_rx_nmea_uc)))
		{
		case NMEA_GPRMC:
		{
			gprmc_t * loc = &nmea_location_rmc;
			NMEA_parse_gprmc((char *) (nmea_rcvr.buffer_rx_nmea_uc), loc);

			if (Flag_enable_NMEA_logging)
				Serial_writeBuf(&Serial_STLK,
						(char*) (nmea_rcvr.buffer_rx_nmea_uc),
						nmea_rcvr.nbr_bytes_rx_ui16);

			if (loc->valid == 'A') //Only when GPRMC is valid
			{
#if 0
				if (loc->lat == 'N')
				{
					geographical_location.lat = NMEA_ANGLE_TO_DEG(loc->latitude);
				}
				else if (loc->lat == 'S')
				{
					geographical_location.lat = -NMEA_ANGLE_TO_DEG(
							loc->latitude);
				}

				if (loc->lon == 'E')
				{
					geographical_location.lon = NMEA_ANGLE_TO_DEG(
							loc->longitude);
				}
				else if (loc->lon == 'W')
				{
					geographical_location.lon = -NMEA_ANGLE_TO_DEG(
							loc->longitude);
				}
#else
				/*
				NMEA_gprmc_lat_degrees(loc,&geographical_location.lat);
				NMEA_gprmc_lon_degrees(loc,&geographical_location.lon);
				*/
#endif

				if (Flag_enable_RTC_GPS_sync)
					RTCUtil_settime(loc->utc);

				if (Flag_enable_LOC_GPS_sync)
				{

					NMEA_gprmc_lat_degrees(loc,&geographical_location.lat);
					NMEA_gprmc_lon_degrees(loc,&geographical_location.lon);

					RTCUtil_saveData(RTC_BKP_DR0,
							(uint8_t*) &geographical_location,
							sizeof(geographical_location));
					StarMap_UpdateLocation(geographical_location);
				}
				//Serial_printf(&Serial_UART2,"GPS %02dh%02dm%02ds %02d/%02d/%02d \r\n LOC lon:%lf lat:%lf\r\n",loc.utc.tm_hour,loc.utc.tm_min,loc.utc.tm_sec,loc.utc.tm_mday,loc.utc.tm_mon,loc.utc.tm_year,starlocation.lon,starlocation.lat);

			}

		}
			break;
		case NMEA_GPGGA:
		{
			gpgga_t * loc = &nmea_location_gga;
			//Serial_writeBuf(&Serial_UART2, (char*) (nmea_rcvr.buffer_rx_nmea_uc),nmea_rcvr.nbr_bytes_rx_ui16);
			NMEA_parse_gpgga((char *) (nmea_rcvr.buffer_rx_nmea_uc), loc);

		}
			break;
		}

		//Trame NMEA traitée
		NMEA_receiver_reset(&nmea_rcvr);
	}
}

void Process_Steppers()
{

	//Processus de suivi de l'etoile actuelle
	//Traiter l'application de la consigne
	//Traiter l'arret du suivi du telescope
	if (Flag_enable_star_follow)
	{
		//Calcul des coordonnees de l'etoiles
		starEqCoords = StarMap_getEqCoords(starSkyCoords);

		StarMap_EqCoords_t eqcoords = starEqCoords;

		//A partir des coordonnes equatoriales, trouver les coordonnées sur les axes des steppers
		/**
		 Angles dans le système:

		 Données:
		 RA: coordonnées celestes de l'astre
		 Dec: coordonnées celeste de l'astre

		 Calculés: On va déterminer les angles via des algorithmes
		 TSL: temps sidéral local calculé a partir des coordonnées GPS et de l'heure UTC
		 Ah: Angle horaire, coordonnée locale de l'astre dans le lieu d'observation
		 Dec: Declinaison coordonnéee locale de l'astre dans le lieu d'observation

		 Angles à appliquer

		 h : angle de l'axe horaire effectif (en prenant en compte le retournement) compté positivement depuis la verticale du telescope vers l'ouest.
		 d : angle de l'axe déclinaison effectif (en prenant en compte le retournement) compté positivement depuis le 0 sur le telescope, pointant l'étoile polaire.
		 d est négatif si il va vers la gauche (OUEST) et d est positif si il va vers la droite (EST)


		 L'angle horaire augmente au cours du temps.
		 On rappelle que l'angle Horaire s'ouvre vers l'OUEST.

		 SI Ah est entre [0;90] ou [270;360[ où bien [-90;90] ALORS
		 d est compté vers l'EST d=Dec
		 SINON
		 d est compté vers l'OUEST d=-Dec
		 **/

		double ah = fmod(eqcoords.ah, 360) + starCorrectionCoords.ra; //Appliquer la correction d'angle
		double dec = fmod(eqcoords.dec, 90) + starCorrectionCoords.dec;

		double h = 0; //angle horaire sur le telescope compté positivement depuis la verticale vers l'ouest
		double d = 0; //angle de l'axe déclinaison effectif (en prenant en compte le retournement) compté positivement depuis le 0 sur le telescope, pointant l'étoile polaire.
		//d est négatif si il va vers la gauche (OUEST) et d est positif si il va vers la droite (EST)
		// -90<= ah <=90
		if ((-90 <= ah && ah <= 90) || (270 <= ah && ah < 360))
		{
			//declinaison compté vers l'EST
			d = dec;
			//Ah sur le telescope ne peut aller que de -90 à +90
			//donc on est bon
			h = ah;
			//maintenant il faut revenir dans les coordonnées du telescope à partir du stopper
			//double angle_from_stopper= ref_angle - angle_from_ref;
		}
		else //Ah est entre 90 et 270
		{
			d = -dec;
			h = ah + 180;
		}

		if (h > 180)
		{
			h -= 360; //retomber dans [-180,180]
		}

		double angle_stepper_ah = stepperAh2.ref_angle + h;
		double angle_stepper_dec = stepperDec2.ref_angle + d;

		if ((sign(d) != sign(last_d))) //Si la déclinaison a été changé cest a dire changé de signe alors il faut retourner !
		{
			Flag_needs_axis_flipping = 1;
		}
		else
		{
			Flag_needs_axis_flipping = 0;
			Flag_authorize_axis_flipping = 0; //Reset the flipping authorization.
		}

		//Si on a le droit de faire le retournement et que l'on doit faire un retournement
		if (!Flag_needs_axis_flipping
				|| (Flag_needs_axis_flipping && Flag_authorize_axis_flipping))
		{

			last_d = d;

			Stepper_goTo(&stepperAh2,
					angle_stepper_ah / stepperAh2.degreesPerSteps);

			Stepper_goTo(&stepperDec2,
					angle_stepper_dec / stepperDec2.degreesPerSteps);

		}

		StarMap_HMS_t hmsah = StarMap_degToHms(eqcoords.ah);

		//Puis demander aux stepper d'aller en position
		Serial_printf(&Serial_STLK,
				"Star ah=%dh%dm%0.2lfs dec=%lf \r\n h=%lf d=%lf", hmsah.h,
				hmsah.m, hmsah.s, eqcoords.dec, h, d);

	}
}

void Process_Main_LED()
{

	if ( //stepperAh2.state == RUNNIN || stepperDec2.state == RUNNIN
	!nmea_rcvr.flag_is_gps_active)
	{
		blinkOnTime = 200;
		blinkOffTime = 100;
	}
	else if (nmea_location_rmc.valid == 'A')
	{
		blinkOnTime = 100;
		blinkOffTime = 100 << 4;
	}
	else
	{
		blinkOnTime = 600;
		blinkOffTime = 100;
	}

	if (blinklevel == HIGH)
	{
		if (millis() - blinkref_time >= blinkOnTime)
		{
			blinkref_time = millis();
			blinklevel = LOW;
			digitalWrite(LD2_PIN, blinklevel);
		}
	}
	else
	{
		if (millis() - blinkref_time >= blinkOffTime)
		{
			blinkref_time = millis();
			blinklevel = HIGH;
			digitalWrite(LD2_PIN, blinklevel);

		}
	}

}

void Process_Buttons()
{
	if (millis() - inputRead_ms >= 10)
	{
		inputRead_ms = millis();
		if (!digitalRead(B1_PIN))
		{

			if (!b1_pressed)
			{
				b1_pressed = 1;
				printf("B1 pressed\r\n");

				Stepper_stopMove(&stepperAh2);
				Stepper_stopMove(&stepperDec2);

				Flag_enable_star_follow = 0;
			}

		}
		else
		{

			if (b1_pressed)
			{
				printf("B1 release\r\n");

			}
			b1_pressed = 0;
		}

	}
}
